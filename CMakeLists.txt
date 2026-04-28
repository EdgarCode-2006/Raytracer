cmake_minimum_required(VERSION 3.14)

# Set compiler and project
set(CMAKE_CXX_COMPILER "g++")
project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_NAME "raytracer")

set(FEATURES_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/rayFeatures")

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.6
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        ceceffd5e8148d4781f5f58cffe1641cf50f8f68
)
FetchContent_MakeAvailable(rlimgui)

# ________________ compile of rest of C++ files.
include_directories(include include/utils)
# --- EXTERNAL DEPENDENCIES (FetchContent) ---

FetchContent_Declare(
  tinyexr
  GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
  GIT_TAG        release
)
FetchContent_MakeAvailable(tinyexr)

# --- COMPILATION OPTIONS ---
# Note: Moved these to specific targets later for better practice
add_compile_options(-fPIC) 

# --- CORE LIBRARY ---
include_directories(include include/utils)

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

add_library(raytracer_core STATIC ${SRC} ${IMGUI_SRC})
target_include_directories(raytracer_core PUBLIC include include/utils ${imgui_SOURCE_DIR} ${rlimgui_SOURCE_DIR} ${tinyexr_SOURCE_DIR})
target_compile_options(raytracer_core PRIVATE -fPIC)

target_link_libraries(raytracer_core PRIVATE tinyexr)

macro(add_ray_feature FEATURE_NAME SRC_PATH)
    add_library(${FEATURE_NAME} SHARED ${SRC_PATH})
    target_link_libraries(${FEATURE_NAME} PRIVATE raytracer_core)
    target_include_directories(${FEATURE_NAME} PRIVATE ${tinyexr_SOURCE_DIR})
    set_target_properties(${FEATURE_NAME} PROPERTIES
        PREFIX "" SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
    )
endmacro()

add_ray_feature(basicLight_feat src/integrators/features/BasicLighting.cpp)
add_ray_feature(SpecularHandler_feat src/integrators/features/SpecularHandler.cpp)
add_ray_feature(AmbientOcclusion_feat src/integrators/features/AmbientOcclusion.cpp)

# --- MAIN EXECUTABLE ---
set(EXECUTABLE_NAME "raytracer")
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
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
        raytracer_core 
        ${RAYLIB_LIB} 
        GL m pthread dl X11 rt
    )
endif()

add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/${EXECUTABLE_NAME}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage_html
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -delete
)

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
)

target_compile_options(raytracer_core PRIVATE -O3 -march=native)
target_compile_options(${EXECUTABLE_NAME} PRIVATE -O3 -march=native)
target_compile_options(basicLight_feat PRIVATE -O3 -march=native)
target_compile_options(SpecularHandler_feat PRIVATE -O3 -march=native)
target_compile_options(AmbientOcclusion_feat PRIVATE -O3 -march=native)