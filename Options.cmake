# build type
set(CMAKE_BUILD_TYPE
    Debug
    CACHE STRING "Choose a build type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                             "MinSizeRel" "RelWithDebInfo")

option(USE_LLD "Use lld instead of ld for linking" OFF)
option(ENABLE_TESTING "Enable Google test" ON)
option(ENABLE_WARNING "Enable compiler warnings" ON)
option(WARNING_AS_ERROR "Change compiler warnings to errors" ON)
option(ENABLE_ASAN "Compile with AddressSanitizer" OFF)

# configure accordingly to options
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    if(MSVC)
        target_compile_options(compile-opts INTERFACE "/Od")
    else(MSVC)
        target_compile_options(compile-opts INTERFACE "-Og")
    endif()
endif()

if(USE_LLD)
    find_program(LLD Lld REQUIRED)
    if(LLD STREQUAL "lld-NOTFOUND")
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
        target_compile_options(compile-opts INTERFACE "/W4")
    else(MSVC)
        target_compile_options(compile-opts
                               INTERFACE "-Wall;-Wextra;-Wformat=2")
    endif()
endif()

if(WARNING_AS_ERROR)
    if(MSVC)
        target_compile_options(compile-opts INTERFACE "/Werror")
    else(MSVC)
        target_compile_options(compile-opts INTERFACE "-Werror")
    endif()
endif()

if(ENABLE_ASAN)
    if(MSVC)
        target_compile_options(compile-opts INTERFACE "/fsanitize=address;/Oy")
    else(MSVC)
        target_compile_options(
            compile-opts INTERFACE "-fsanitize=address;-fno-omit-frame-pointer")
    endif()
endif()
