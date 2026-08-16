# Contributing

BioDynaMo welcomes focused fixes, features, tests, and documentation.

## Build

Install the platform prerequisites, then configure a lightweight development
build from the repository root:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Dparaview=OFF
cmake --build build --parallel
```

ParaView support uses a system installation. Enable it with
`-Dparaview=ON` when the change affects visualization. On Ubuntu, install
`paraview`, `paraview-dev`, `libdouble-conversion-dev`, and `libutfcpp-dev`.

## Test

Run the complete unit suite before submitting a pull request:

```bash
cmake --build build --target run-unit-tests
```

Run the repository-owned formatting and copyright checks:

```bash
cmake --build build --target check-format-all
./util/housekeeping/check-copyright.sh \
  util/housekeeping/copyright_cpp.txt \
  $(./util/housekeeping/get-all-src-files.sh .)
```

Add a regression test for changed behavior. Keep commits focused and explain
the observed problem, the change, and the verified result in the pull request.

The [developer guide](https://biodynamo.org/docs/devguide/contribute/)
contains the complete contribution workflow.
