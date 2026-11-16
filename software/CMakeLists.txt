cmake_minimum_required(VERSION 3.22)

project(unwired LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(UNWIRED_PLATFORM "desktop" CACHE STRING "Platform directory under src/platforms")

set(UNWIRED_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/${UNWIRED_PLATFORM}/main.c")

if(UNWIRED_PLATFORM STREQUAL "desktop")
    set(UNWIRED_TARGET unwired-desktop)
endif()

add_executable(${UNWIRED_TARGET}
	${UNWIRED_MAIN}
)

target_include_directories(${UNWIRED_TARGET} PRIVATE
	src
)

target_compile_features(${UNWIRED_TARGET} PRIVATE c_std_23)

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
