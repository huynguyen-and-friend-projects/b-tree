# build type
set(CMAKE_BUILD_TYPE
    Debug
    CACHE STRING "Choose a build type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                             "MinSizeRel" "RelWithDebInfo")

option(USE_CCACHE "Use ccache" OFF)
option(USE_LLD "Use lld instead of ld for linking" OFF)
option(ENABLE_PCH "Enable precompiled header" ON)
option(ENABLE_TESTING "Enable Google test" ON)
option(ENABLE_WARNING "Enable compiler warnings" ON)
option(WARNING_AS_ERROR "Change compiler warnings to errors" ON)
option(ENABLE_ASAN "Compile with AddressSanitizer" OFF)

# configure accordingly to options
if(ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if(NOT CCACHE)
        message("Cannot find ccache")
    else()
        message("Found ccache and is using it")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    endif()
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    if(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "/Od")
    else(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "-O0")
    endif()
endif()

if(USE_LLD)
    find_program(LLD lld REQUIRED)
    if(NOT LLD)
        message(
            FATAL_ERROR
                "Error, LLD not found, please either turn off option USE_LLD or add directory of LLD to PATH"
        )
    endif()
    set(CMAKE_LINKER lld)
endif()

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()
if(ENABLE_WARNING)
    if(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "/W4")
    else(MSVC)
        target_compile_options(
            b-tree-compile-opts
            INTERFACE "-Wall;-Wextra;-Wformat=2;-fdiagnostics-color=always")
    endif()
endif()

if(WARNING_AS_ERROR)
    if(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "/WX")
    else(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "-Werror")
    endif()
endif()

if(ENABLE_ASAN)
    if(MSVC)
        target_compile_options(b-tree-compile-opts INTERFACE "/fsanitize=address;/Oy")
    else(MSVC)
        target_compile_options(
            b-tree-compile-opts INTERFACE "-fsanitize=address;-fno-omit-frame-pointer")
    endif()
endif()
