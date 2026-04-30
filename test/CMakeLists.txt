cmake_minimum_required(VERSION 3.14)
project(raytracer_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(PROJECT_INCLUDE "${PROJECT_ROOT}/include")

set(TEST_SOURCES
    test_vector3d.cpp
    test_triangle.cpp
    test_ray.cpp
    test_aabb.cpp
    test_rayhit.cpp
    test_material.cpp
    test_matrix4.cpp
    ${PROJECT_ROOT}/src/opti/BVH/AABB.cpp
    ${PROJECT_ROOT}/src/opti/BVH/BVHNode.cpp
    ${PROJECT_ROOT}/src/utils/Matrix4.cpp
)

add_executable(raytracer_tests ${TEST_SOURCES})

target_include_directories(raytracer_tests PRIVATE
    ${PROJECT_INCLUDE}
    ${PROJECT_INCLUDE}/utils
)

target_link_libraries(raytracer_tests PRIVATE GTest::gtest_main)

target_compile_options(raytracer_tests PRIVATE -O0 -g -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(raytracer_tests)
