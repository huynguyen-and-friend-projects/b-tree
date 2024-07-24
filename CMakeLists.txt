cmake_minimum_required(VERSION 3.28)
project(
    b-tree-proto
    VERSION 0.1.0
    LANGUAGES C CXX)
# use C++23 requires at least C++20
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
# turn off compiler extensions
if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
# generate compile_commands.json
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# enforce standards on all compiled pieces
add_library(b-tree-compile-opts INTERFACE)
if(NOT CMAKE_CXX_STANDARD)
    target_compile_features(b-tree-compile-opts INTERFACE cxx_std_23)
else()
    target_compile_features(b-tree-compile-opts
                            INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
endif()

if(MSVC)
    target_compile_options(b-tree-compile-opts INTERFACE "/Za")
else(MSVC)
    target_compile_options(b-tree-compile-opts INTERFACE "-pedantic")
endif()

include(Options.cmake)

add_subdirectory(src)
