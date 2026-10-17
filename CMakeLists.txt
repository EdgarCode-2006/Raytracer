cmake_minimum_required(VERSION 3.14)

project(raytracer LANGUAGES CXX)

# --- DEFAULT BUILD TYPE ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- GLOBAL CONFIGURATION ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_NAME "client")
set(FEATURES_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/rayFeatures")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- PLATFORM ---
if(WIN32)
    set(FEATURE_SUFFIX ".dll")
    set(PLATFORM_LIBS opengl32 gdi32 winmm)
    add_compile_definitions(_USE_MATH_DEFINES) # M_PI with MSVC
else()
    set(FEATURE_SUFFIX ".so")
    set(PLATFORM_LIBS GL m pthread dl X11 rt)
endif()

# MinGW: make client.exe and the features only depend on Windows itself, so they can be
# copied to any Windows machine. The GCC runtime is linked statically, and raylib and
# libconfig++ are built from source instead of using the installed ones.
set(SELF_CONTAINED_BUILD OFF)
if(MINGW)
    option(RAYTRACER_STATIC_RUNTIME "Build self-contained Windows binaries" ON)
    set(SELF_CONTAINED_BUILD ${RAYTRACER_STATIC_RUNTIME})
endif()

# --- DEPENDENCIES ---
include(FetchContent)

# Builds a declared dependency from source as a static library, without its examples and tests.
function(fetch_static_dependency NAME)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # let option() in the dependency take the values below
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_EXAMPLES OFF)
    set(BUILD_TESTS OFF)
    FetchContent_MakeAvailable(${NAME})
endfunction()

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
)

FetchContent_Declare(rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG ceceffd5e8148d4781f5f58cffe1641cf50f8f68
)

FetchContent_Declare(tinyexr
    GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
    GIT_TAG release
)

FetchContent_MakeAvailable(imgui rlimgui tinyexr stb)

# --- GAME LIBRARIES ---
# What the client and the server share (inputs, world state, entity types), and the game rules
# the server runs. Neither knows about the raytracer, raylib or ImGui: only their own headers
# are on their include path.
file(GLOB_RECURSE COMMON_SRC "common/src/*.cpp")
add_library(rtype_common STATIC ${COMMON_SRC})
target_include_directories(rtype_common PUBLIC "${CMAKE_SOURCE_DIR}/common/include")

file(GLOB_RECURSE SIMULATION_SRC "simulation/src/*.cpp")
add_library(rtype_simulation STATIC ${SIMULATION_SRC})
target_include_directories(rtype_simulation PUBLIC "${CMAKE_SOURCE_DIR}/simulation/include")
target_link_libraries(rtype_simulation PUBLIC rtype_common)

# --- SOURCE FILES ---
file(GLOB_RECURSE SRC "src/*.cpp")
file(GLOB_RECURSE FEATURE_SRC "src/integrators/features/*.cpp")

list(REMOVE_ITEM SRC ${FEATURE_SRC})
list(REMOVE_ITEM SRC "${CMAKE_SOURCE_DIR}/src/main.cpp")

set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

# --- CORE LIB ---
add_library(raytracer_core STATIC ${SRC} ${IMGUI_SRC})

target_include_directories(raytracer_core PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/utils"
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${tinyexr_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

target_link_libraries(raytracer_core PRIVATE tinyexr)

if(TARGET tinyexr)
    target_compile_options(tinyexr PRIVATE -w)
endif()

if(SELF_CONTAINED_BUILD)
    # Propagated to everything linking the core: the executable and every feature.
    target_link_options(raytracer_core INTERFACE -static)
endif()

# --- FEATURE MACRO ---
set(RAY_FEATURES "")

macro(add_ray_feature FEATURE_NAME SRC_PATH)
    add_library(${FEATURE_NAME} SHARED ${SRC_PATH})

    target_link_libraries(${FEATURE_NAME} PRIVATE raytracer_core)

    # On Windows a .dll is a RUNTIME output (LIBRARY_OUTPUT_DIRECTORY is ignored), and the
    # $<1:> stops Visual Studio from adding a Release/ or Debug/ subfolder.
    # WINDOWS_EXPORT_ALL_SYMBOLS makes MSVC export getInstance, MinGW already does it.
    set_target_properties(${FEATURE_NAME} PROPERTIES
        PREFIX ""
        SUFFIX "${FEATURE_SUFFIX}"
        LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "$<1:${FEATURES_OUTPUT_DIR}>"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    list(APPEND RAY_FEATURES ${FEATURE_NAME})
endmacro()

add_ray_feature(basicLight_feat src/integrators/features/BasicLighting.cpp)
add_ray_feature(SpecularHandler_feat src/integrators/features/SpecularHandler.cpp)
add_ray_feature(AmbientOcclusion_feat src/integrators/features/AmbientOcclusion.cpp)
add_ray_feature(Emission_feat src/integrators/features/Emission.cpp)
add_ray_feature(PathTracing_feat src/integrators/features/PathTracing.cpp)
add_ray_feature(ShadeSmooth_feat src/integrators/features/ShadeSmooth.cpp)
add_ray_feature(NormalMap_feat src/integrators/features/debug/normalMap.cpp)
add_ray_feature(OutLine_feat src/integrators/features/debug/OutLine.cpp)
add_ray_feature(WireFrame_feat src/integrators/features/debug/WireFrame.cpp)
add_ray_feature(RayTraverseDepth_feat src/integrators/features/debug/RayTraverseDepth.cpp)

# --- EXECUTABLE ---
add_executable(${EXECUTABLE_NAME} src/main.cpp)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE raytracer_core)

# The features are loaded at runtime from rayFeatures/, build them along with the executable.
add_dependencies(${EXECUTABLE_NAME} ${RAY_FEATURES})

# --- LIBCONFIG++ ---
if(NOT SELF_CONTAINED_BUILD)
    find_library(LIBCONFIGPP_LIB NAMES config++ libconfig++)
    find_path(LIBCONFIGPP_INC libconfig.h++)
endif()

if(NOT SELF_CONTAINED_BUILD AND LIBCONFIGPP_LIB AND LIBCONFIGPP_INC)
    target_include_directories(raytracer_core PUBLIC ${LIBCONFIGPP_INC})
    target_link_libraries(raytracer_core PUBLIC ${LIBCONFIGPP_LIB})
    target_include_directories(rtype_simulation PRIVATE ${LIBCONFIGPP_INC})
    target_link_libraries(rtype_simulation PRIVATE ${LIBCONFIGPP_LIB})
else()
    message(STATUS "Building libconfig++ from source")
    FetchContent_Declare(libconfig
        GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git
        GIT_TAG        v1.8.2
        GIT_SHALLOW    TRUE
    )
    fetch_static_dependency(libconfig)

    if(MSVC)
        target_link_libraries(raytracer_core PUBLIC libconfig++)
        target_link_libraries(rtype_simulation PRIVATE libconfig++)
    else()
        target_link_libraries(raytracer_core PUBLIC config++)
        target_link_libraries(rtype_simulation PRIVATE config++)
    endif()
endif()

# --- RAYLIB ---
if(NOT SELF_CONTAINED_BUILD)
    find_library(RAYLIB_LIB raylib)
    find_path(RAYLIB_INC raylib.h)
endif()

if(NOT SELF_CONTAINED_BUILD AND RAYLIB_LIB AND RAYLIB_INC)
    target_include_directories(raytracer_core PUBLIC ${RAYLIB_INC})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_LIB} ${PLATFORM_LIBS})
else()
    message(STATUS "Building raylib from source")
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        6.0
        GIT_SHALLOW    TRUE
    )
    fetch_static_dependency(raylib)

    # Only the executable links raylib, the core just needs its headers.
    target_include_directories(raytracer_core PUBLIC $<TARGET_PROPERTY:raylib,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE raylib ${PLATFORM_LIBS})
endif()

target_compile_definitions(raytracer_core PUBLIC "IsTextureReady=IsTextureValid")

# --- COMPILATION FLAGS ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC AND NOT MINGW) # no AddressSanitizer with MinGW
        add_compile_options(-g -O0 -fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
else()
    if(NOT MSVC)
        add_compile_options(
            -O3
            -march=native
            -DNDEBUG
        )
    endif()

    option(ENABLE_OPENMP "Enable OpenMP" ON)
    if(ENABLE_OPENMP)
        find_package(OpenMP)
        if(OpenMP_CXX_FOUND)
            target_link_libraries(raytracer_core PUBLIC OpenMP::OpenMP_CXX)
            target_link_libraries(${EXECUTABLE_NAME} PUBLIC OpenMP::OpenMP_CXX)
        endif()
    endif()
endif()

# --- CLEAN ---
add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/${EXECUTABLE_NAME}
)

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
)