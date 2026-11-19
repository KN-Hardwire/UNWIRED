cmake_minimum_required(VERSION 3.22)

project(unwired LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(UNWIRED_PLATFORM "desktop" CACHE STRING "Platform directory under src/platforms")

set(UNWIRED_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/${UNWIRED_PLATFORM}/main.c")

if(UNWIRED_PLATFORM STREQUAL "desktop")
    set(UNWIRED_TARGET unwired-desktop)

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL/CMakeLists.txt")
        message(FATAL_ERROR
            "SDL3 submodule is missing. From the repository root, run: "
            "git submodule update --init --recursive software/third_party/SDL")
    endif()

    set(SDL_SHARED OFF CACHE BOOL "Build SDL as a shared library")
    set(SDL_STATIC ON CACHE BOOL "Build SDL as a static library")
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL's test support library")
    set(SDL_TESTS OFF CACHE BOOL "Build SDL's tests")
    set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL's examples")
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "Disable SDL installation targets")
    add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)
endif()

add_executable(${UNWIRED_TARGET}
	${UNWIRED_MAIN}
)

target_include_directories(${UNWIRED_TARGET} PRIVATE
	src
)

target_compile_features(${UNWIRED_TARGET} PRIVATE c_std_23)

if(UNWIRED_PLATFORM STREQUAL "desktop")
    target_link_libraries(${UNWIRED_TARGET} PRIVATE SDL3::SDL3)
endif()

# Compile with warnings
if(MSVC)
    target_compile_options(${UNWIRED_TARGET} PRIVATE /W4)
else()
    target_compile_options(${UNWIRED_TARGET} PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Wconversion
		-Wshadow
	)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(unwired-tests tests/placeholder.c)
    target_compile_features(unwired-tests PRIVATE c_std_23)
    add_test(NAME placeholder COMMAND unwired-tests)
endif()
