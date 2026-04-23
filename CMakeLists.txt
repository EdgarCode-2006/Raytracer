cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_COMPILER "g++")
project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_NAME "raytracer")

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

include_directories(include include/utils)
file(GLOB_RECURSE SRC "src/*.cpp")

set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

add_executable(${EXECUTABLE_NAME} ${SRC} ${IMGUI_SRC})

find_library(RAYLIB_LIB raylib)
find_path(RAYLIB_INC raylib.h)
if(RAYLIB_LIB AND RAYLIB_INC)
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${RAYLIB_INC}
        ${imgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
    )
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE "IsTextureValid=IsTextureReady")
    target_link_libraries(${EXECUTABLE_NAME} ${RAYLIB_LIB} GL m pthread dl X11 rt)
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
