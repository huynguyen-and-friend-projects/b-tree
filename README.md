# B tree prototype

## What

- This is an experimental B-tree for the smoldb project.

## How to build

- Make sure to have the following installed:
  - CMake
  - Any "common" C/C++ compiler (gcc, clang. MSVC may work)

- These are optional, but nice to have:
  - libasan (enabled with ENABLE_ASAN option)
  - lld (enabled with USE_LLD option)
  - cmake-gui or ccmake (makes life easier)

- If you use cmake-gui, simply point the source directory to this project root.
The build directory should be {project-root}/build. After that, click "Configure"
then "Generate." Finally, run `cmake --build build`.

- If you use cmake (the command line), do as follow:

```bash
# create build directory
mkdir build && cd build
# configure the build
cmake [options] ..
# options include these following defaults:
# -DCMAKE_BUILD_TYPE=Debug (or Release, MinSizeRel, RelWithDebInfo)
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON (export a compile_commands.json file)
# -DUSE_LLD=OFF (use lld instead of the default linker)
# -DENABLE_TESTING=ON (create a test executable)
# -DENABLE_ASAN=OFF (use libasan to catch memory errors)
# -DENABLE_WARNING=ON (misleading name; enable extra warnings)
# -DWARNINGS_AS_ERRORS=ON (turn all compiler warnings to errors)
cmake --build .
cd ..
sh ./run-test.sh # run test
# note, you can only run test if you enable testing (which, is enabled by default)
```
