cmake_minimum_required(VERSION 3.14)

# Set compiler and project
set(CMAKE_CXX_COMPILER "g++")
project(nanotekspice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- EXTERNAL DEPENDENCIES (FetchContent) ---
include(FetchContent)

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

# Main raytracer engine
add_library(raytracer_core STATIC ${SRC})

target_include_directories(raytracer_core PUBLIC 
    include 
    include/utils 
    ${tinyexr_SOURCE_DIR}
)
target_link_libraries(raytracer_core PRIVATE tinyexr)

# --- FEATURE PLUGINS (Shared Libraries) ---
set(FEATURES_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/rayFeatures")

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
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_INC})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_LIB} GL m pthread dl X11 rt)
else()
    message(FATAL_ERROR "raylib not found")
endif()

set(OPTIM_FLAGS -O3 -flto -march=native)
target_compile_options(${EXECUTABLE_NAME} PRIVATE ${OPTIM_FLAGS})
target_compile_options(raytracer_core PRIVATE ${OPTIM_FLAGS})
target_compile_options(basicLight_feat PRIVATE ${OPTIM_FLAGS})
target_compile_options(SpecularHandler_feat PRIVATE ${OPTIM_FLAGS})
target_compile_options(AmbientOcclusion_feat PRIVATE ${OPTIM_FLAGS})
