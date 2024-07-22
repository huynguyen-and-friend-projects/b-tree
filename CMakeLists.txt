cmake_minimum_required(VERSION 3.28)
project(
    b-tree-proto
    VERSION 0.1.0
    LANGUAGES C CXX)
# use C23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# turn off compiler extensions
set(CMAKE_CXX_EXTENSIONS OFF)
# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enforce standards on all compiled pieces
add_library(b-tree-compile-opts INTERFACE)
target_compile_features(b-tree-compile-opts INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
if(MSVC)
    target_compile_options(b-tree-compile-opts INTERFACE "/Za")
else(MSVC)
    target_compile_options(b-tree-compile-opts INTERFACE "-pedantic")
endif()

include(Options.cmake)

add_subdirectory(src)
