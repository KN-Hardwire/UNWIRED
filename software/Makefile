.DEFAULT_GOAL := desktop

CMAKE ?= cmake
BUILD_DIR ?= build
CMAKE_ARGS ?=

.PHONY: build desktop firmware test help

help:
	@printf '%s\n' \
		'Usage: make [target] [VARIABLE=value ...]' \
		'' \
		'Targets:' \
		'  desktop   Build the desktop executable (default target).' \
		'  build     Alias for desktop.' \
		'  firmware  Show a warning: STM32 firmware build is unavailable.' \
		'  test      Build tests and run them through CTest.' \
		'  help      Show this help message.' \
		'' \
		'Variables:' \
		'  BUILD_DIR   Parent build directory (default: build).' \
		'  CMAKE       CMake executable (default: cmake).' \
		'  CMAKE_ARGS  Additional CMake configuration options.'

build: desktop

desktop:
	$(CMAKE) -S . -B $(BUILD_DIR)/desktop -DUNWIRED_PLATFORM=desktop -DBUILD_TESTING=OFF $(CMAKE_ARGS)
	$(CMAKE) --build $(BUILD_DIR)/desktop

firmware:
	@printf '%s\n' 'Warning: firmware build is unavailable because STM32 platform code is not implemented yet.'

test:
	$(CMAKE) -S . -B $(BUILD_DIR)/tests -DUNWIRED_PLATFORM=desktop -DBUILD_TESTING=ON $(CMAKE_ARGS)
	$(CMAKE) --build $(BUILD_DIR)/tests --target unwired-tests
	cd $(BUILD_DIR)/tests && ctest --output-on-failure
