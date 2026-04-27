cmake_minimum_required(VERSION 3.14)

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

file(GLOB_RECURSE SRC
    "src/*.cpp"
)

file(GLOB_RECURSE FEATURE_SRC
    "src/integrators/features/*.cpp"
)

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
target_include_directories(raytracer_core PUBLIC include include/utils ${imgui_SOURCE_DIR} ${rlimgui_SOURCE_DIR})
target_compile_options(raytracer_core PRIVATE -fPIC)

# --- link plugins to core lib.
add_library(basicLight_feat SHARED src/integrators/features/BasicLighting.cpp)
target_link_libraries(basicLight_feat PRIVATE raytracer_core)

set_target_properties(basicLight_feat PROPERTIES
    PREFIX "" SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
)

add_library(SpecularHandler_feat SHARED src/integrators/features/SpecularHandler.cpp)
target_link_libraries(SpecularHandler_feat PRIVATE raytracer_core)

set_target_properties(SpecularHandler_feat PROPERTIES
    PREFIX "" SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
)

add_library(AmbientOcclusion_feat SHARED src/integrators/features/AmbientOcclusion.cpp)
target_link_libraries(AmbientOcclusion_feat PRIVATE raytracer_core)

set_target_properties(AmbientOcclusion_feat PROPERTIES
    PREFIX "" SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${FEATURES_OUTPUT_DIR}"
)

add_executable(${EXECUTABLE_NAME} src/main.cpp)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE raytracer_core basicLight_feat SpecularHandler_feat AmbientOcclusion_feat)

find_library(RAYLIB_LIB raylib)
find_path(RAYLIB_INC raylib.h)
if(RAYLIB_LIB AND RAYLIB_INC)
    target_include_directories(raytracer_core PUBLIC ${RAYLIB_INC})
    target_compile_definitions(raytracer_core PUBLIC "IsTextureValid=IsTextureReady")
    target_link_libraries(raytracer_core PUBLIC ${RAYLIB_LIB} GL m pthread dl X11 rt)
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_INC})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RAYLIB_LIB} GL m pthread dl X11 rt)
else()
    message(FATAL_ERROR "raylib not found")
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CRITERION criterion)
endif()

if(CRITERION_FOUND)
    file(GLOB_RECURSE TEST_SRC "tests/*.cpp")
    file(GLOB_RECURSE SRC_NO_MAIN "src/*.cpp")
    list(FILTER SRC_NO_MAIN EXCLUDE REGEX ".*main\\.cpp$")

    add_executable(unit_tests EXCLUDE_FROM_ALL ${SRC_NO_MAIN} ${TEST_SRC})
    target_include_directories(unit_tests PRIVATE include tests)
    target_link_libraries(unit_tests ${CRITERION_LIBRARIES})
    target_compile_options(unit_tests PRIVATE ${CRITERION_CFLAGS_OTHER} --coverage)
    target_link_options(unit_tests PRIVATE --coverage)

    enable_testing()
    add_test(NAME unit_tests COMMAND unit_tests)

    add_custom_target(tests
        COMMAND unit_tests --verbose
        DEPENDS unit_tests
    )

    find_program(GCOVR gcovr)
    if(GCOVR)
        set(GCOVR_COMMON
            --gcov-executable "llvm-cov gcov"
            --root ${CMAKE_SOURCE_DIR}
            --filter ${CMAKE_SOURCE_DIR}/src/
            --object-directory ${CMAKE_BINARY_DIR}
        )

        add_custom_target(coverage
            COMMAND unit_tests
            COMMAND ${GCOVR} ${GCOVR_COMMON} --txt
            DEPENDS unit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(coverage-html
            COMMAND unit_tests
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_html
            COMMAND ${GCOVR} ${GCOVR_COMMON}
                    --html-details ${CMAKE_BINARY_DIR}/coverage_html/index.html
            DEPENDS unit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
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
