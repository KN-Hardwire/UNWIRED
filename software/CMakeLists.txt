cmake_minimum_required(VERSION 3.22)

project(unwired-firmware LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(unwired-firmware
	src/main.c
)

target_include_directories(unwired-firmware PRIVATE
	include
)

target_compile_features(unwired-firmware PRIVATE c_std_23)

# Compile with warnings
if(MSVC)
    target_compile_options(unwired-firmware PRIVATE /W4)
else()
    target_compile_options(unwired-firmware PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Wconversion
		-Wshadow
	)
endif()
