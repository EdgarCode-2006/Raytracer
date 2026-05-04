cmake_minimum_required(VERSION 3.14)

project(raytracer LANGUAGES CXX)

# --- GLOBAL CONFIGURATION ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_NAME "raytracer")
set(FEATURES_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/rayFeatures")

# Optimization and Position Independent Code (needed for shared libs)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- DEPENDENCIES (FetchContent) ---
include(FetchContent)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.6)
FetchContent_Declare(rlimgui GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git GIT_TAG ceceffd5e8148d4781f5f58cffe1641cf50f8f68)
FetchContent_Declare(tinyexr GIT_REPOSITORY https://github.com/syoyo/tinyexr.git GIT_TAG release)

FetchContent_MakeAvailable(imgui rlimgui tinyexr)

# --- SOURCE FILE MANAGEMENT ---
file(GLOB_RECURSE SRC "src/*.cpp")
file(GLOB_RECURSE FEATURE_SRC "src/integrators/features/*.cpp")

# Exclude specific files from the core library to avoid multiple definitions
list(REMOVE_ITEM SRC ${FEATURE_SRC})
list(REMOVE_ITEM SRC "${CMAKE_SOURCE_DIR}/src/main.cpp")

set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

# --- TARGETS ---

# 1. Core Library
add_library(raytracer_core STATIC ${SRC} ${IMGUI_SRC})

# THIS IS THE FIX: We use PUBLIC so anyone linking to raytracer_core 
# (like your features or main) automatically sees these folders.
target_include_directories(raytracer_core PUBLIC 
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/utils"
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${tinyexr_SOURCE_DIR}
)

target_link_libraries(raytracer_core PRIVATE tinyexr)

# 2. Feature Plugin Macro
macro(add_ray_feature FEATURE_NAME SRC_PATH)
    add_library(${FEATURE_NAME} SHARED ${SRC_PATH})
    # Linking to raytracer_core now automatically provides the includes
    target_link_libraries(${FEATURE_NAME} PRIVATE raytracer_core)
    
    set_target_properties(${FEATURE_NAME} PROPERTIES
        PREFIX "" 
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
    )
endmacro()

add_ray_feature(basicLight_feat src/integrators/features/BasicLighting.cpp)
add_ray_feature(SpecularHandler_feat src/integrators/features/SpecularHandler.cpp)
add_ray_feature(AmbientOcclusion_feat src/integrators/features/AmbientOcclusion.cpp)

# 3. Main Executable
add_executable(${EXECUTABLE_NAME} src/main.cpp)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
    raytracer_core 
    basicLight_feat 
    SpecularHandler_feat 
    AmbientOcclusion_feat
)

# --- RAYLIB & SYSTEM LIBS ---
find_library(RAYLIB_LIB raylib)
find_path(RAYLIB_INC raylib.h)

if(RAYLIB_LIB AND RAYLIB_INC)
    target_include_directories(raytracer_core PUBLIC ${RAYLIB_INC})
    target_compile_definitions(raytracer_core PUBLIC "IsTextureReady=IsTextureValid")
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_LIB} GL m pthread dl X11 rt)
else()
    message(FATAL_ERROR "raylib not found")
endif()

# --- COMPILATION OPTIONS (Debug vs Release) ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(raytracer_core PUBLIC -g -O0 -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${EXECUTABLE_NAME} PRIVATE -fsanitize=address)
else()
    # Apply optimizations to all targets linking to core
    target_compile_options(raytracer_core PUBLIC -O3 -march=native)
endif()

# --- UTILITIES ---
add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/${EXECUTABLE_NAME}
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -delete
)

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
)