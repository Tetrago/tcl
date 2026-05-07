# Template Container Library

The Template Container Library (TCL) exists to provide experience in
implementing reliable, safe, and ergonomic containers in modern C++. Raw
pointers are intentional.

## Components

- `forward_list`

## Testing

- Tests are enabled with `TCL_BUILD_TEST`.
- Coverage reports are enabled with `TCL_ENABLE_COVERAGE`. Clang, llvm-cov, and llvm-profdata are required.
- Address sanitization is enabled with `TCL_ENABLE_ASAN` on GCC or Clang.
