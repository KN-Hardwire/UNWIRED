# UNWIRED Software

## Prerequisites

- CMake 3.22 or newer
- Make
- A C compiler with C23 support

CTest is included with CMake and runs the tests.

## Building

From the `software` directory:

```sh
make desktop  # Build the desktop simulator (default; also make build)
make firmware # Build the flashable firmware binary
make test     # Build and run tests through CTest
make help     # List all targets and build variables
```

Override `BUILD_DIR`, `CMAKE`, or `CMAKE_ARGS` as needed, for example:

```sh
make desktop BUILD_DIR=build-release CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"
```

Use a fresh build directory when changing compilers or toolchain files.

## Development

Follow the project documentation when adding or changing source code:

- [Software Architecture](docs/architecture.md) defines layer responsibilities,
  dependency rules and application data flow.
- [Repository Structure](docs/repo_structure.md) is the source of truth for
  directory paths and where code belongs.

Keep these documents up to date when changing the architecture or repository
layout, and keep the implementation consistent with them.

- Match the surrounding C style and resolve compiler warnings introduced by changes.
- Add or update tests for changes to domain logic and application behavior.
- Run `make desktop` and `make test` before submitting changes.
