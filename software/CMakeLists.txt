cmake_minimum_required(VERSION 3.22)

project(unwired LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(UNWIRED_PLATFORM "desktop" CACHE STRING "Platform directory under src/platforms")

set(UNWIRED_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/${UNWIRED_PLATFORM}/main.c")

if(UNWIRED_PLATFORM STREQUAL "desktop")
    set(UNWIRED_TARGET unwired-desktop)

    # PAL implementations for this platform, excluding the entry point.
    set(UNWIRED_PLATFORM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/desktop/desktop_matrix.c"
    )

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
	${UNWIRED_PLATFORM_SOURCES}
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
    set(UNWIRED_WARNING_OPTIONS /W4)
else()
    set(UNWIRED_WARNING_OPTIONS
		-Wall
		-Wextra
		-Wpedantic
		-Wconversion
		-Wshadow
	)
endif()

target_compile_options(${UNWIRED_TARGET} PRIVATE ${UNWIRED_WARNING_OPTIONS})

include(CTest)
if(BUILD_TESTING)
    # Tests compile the platform sources under test directly; no entry point,
    # no SDL.
    add_executable(unwired-tests
		tests/pal_matrix_desktop_test.c
		src/platforms/desktop/desktop_matrix.c
	)
    target_include_directories(unwired-tests PRIVATE src)
    target_compile_features(unwired-tests PRIVATE c_std_23)
    target_compile_options(unwired-tests PRIVATE ${UNWIRED_WARNING_OPTIONS})
    add_test(NAME pal-matrix-desktop COMMAND unwired-tests)
endif()
