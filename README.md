# B tree prototype

## What

- This is an experimental B-tree for the smoldb project.

## How to build

- Make sure to have the following installed:
  - CMake
  - Any "common" C/C++ compiler (gcc, clang. MSVC may work)

- These are optional, but nice to have:
  - libASan (enabled with ENABLE_ASAN option)
  - UBSan, msan (enabled with ENABLE_UBSAN and ENABLE_MSAN respectively)
  - TSan (may be added later?)
  - hwasan (if your device supports it)
  - libFuzzer (for fuzz testing)
  - gcov (truth be told, the author still hasn't used it)
  - googletest (if you don't have it, CMake downloads it for you)
  - lld (enabled with USE_LLD option)
  - libc++ (to be fair, either it's the default on your system, or it doesn't work)
  - cmake-gui or ccmake (makes life easier)

- If you use cmake-gui, simply point the source directory to this project root.
The build directory should be {project-root}/build. After that, click "Configure"
then "Generate." Finally, run `cmake --build build`.
- If you use ccmake, do as follow:

```bash
ccmake -B <your build directory> -S <this project root> [-G <your preferred generator>]\
    [-DCMAKE_CXX_EXPORT_COMPILE_COMMANDS=ON]\
    [-DCMAKE_CXX_COMPILER=<your preferred compiler>]
# of course, if you have exported these as environment variables,
# there's no need to manually add in like this.
```

- If you use cmake (the command line), do as follow:

```bash
# create build directory
mkdir build && cd build
# configure the build
cmake [options] ..
# options include these following defaults:
# -DCMAKE_BUILD_TYPE=Debug (or Release, MinSizeRel, RelWithDebInfo)
# -DUSE_CCACHE=OFF (use ccache to save some artifacts)
# -DUSE_LLD=OFF (use lld instead of the default linker)
# -DUSE_LIBCXX=OFF (gcc/clang on Linux specifically; use libc++ instead of stdlibc++.
# WARNING: VERY UNSTABLE)
# -DENABLE_PCH=ON (enable precompiled headers)
# -DENABLE_TESTING=ON (create a test executable)
# -DENABLE_WARNING=ON (enable extra warnings)
# -DWARNINGS_AS_ERRORS=ON (turn all compiler warnings to errors)
# -DENABLE_ASAN=OFF (use libasan to catch memory errors)
# -DENABLE_UBSAN=OFF (use ubsan to catch undefined behaviours)
# -DENABLE_MSAN=OFF (use libmsan to catch sussy memory usage)
# -DENABLE_COVERAGE=OFF (compile with coverage flags for use with, say, gcov)
# -DENABLE_FUZZ=OFF (compile fuzz test)
cmake --build .
cd ..
sh ./run-test.sh # run test
# note, you can only run test if you enable testing (which, is enabled by default)
```

## Run tests

- To run unit test, simply run:

```bash
# make sure you have turned on ENABLE_TESTING and built the project
ctest --test-dir <your build dir>/test
```

- To run fuzz test, run the executable:
