# if build type is not set yet
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("No build type is configured! Default to Debug")
    set(CMAKE_BUILD_TYPE
        "Debug"
        CACHE STRING
              "Choose a build type (Debug, Release, MinSizeRel, RelWithDebInfo)"
              FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE
                 PROPERTY STRINGS "Debug;Release;MinSizeRel;RelWithDebInfo")
endif()

option(btree_USE_CCACHE "Use ccache" OFF)
option(btree_USE_LLD "Use lld instead of ld for linking" OFF)
option(btree_USE_LIBCXX "Use libcxx instead of stdlibcxx" OFF)
option(btree_btree_ENABLE_OPTIMIZATION
       "Add some optimization flags. Maybe useful only when build type is Debug"
       ON)
option(btree_ENABLE_PCH "Enable precompiled header" ON)
option(btree_ENABLE_TESTING "Enable Google test" ON)
option(btree_ENABLE_WARNING "Enable compiler warnings" ON)
option(btree_WARNING_AS_ERROR "Change compiler warnings to errors" ON)
option(btree_ENABLE_ASAN "Compile with AddressSanitizer" OFF)
option(btree_ENABLE_UBSAN "Compile with UndefinedBehaviorSanitizer" OFF)
option(btree_ENABLE_MSAN "Compile with MemorySanitizer" OFF)
option(btree_ENABLE_COVERAGE "Compile with coverage flag" OFF)
option(btree_ENABLE_FUZZ
       "Enable fuzz testing. Currently only working with clang" OFF)
option(
    btree_CLANGD_COMPAT
    "Enable supposedly unnecessary compile flags for the b-tree target, mainly so that clangd doesn't throw a bunch of false positives"
    OFF)

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    mark_as_advanced(FORCE btree_ENABLE_OPTIMIZATION)
else()
    mark_as_advanced(CLEAR btree_ENABLE_OPTIMIZATION)
endif()

# configure accordingly to options
if(btree_ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if(NOT CCACHE)
        message("Cannot find ccache")
    else()
        message("Found ccache and is using it")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    endif()
endif()

if(btree_ENABLE_OPTIMIZATION)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        if(MSVC)
            target_compile_options(btree_compile_opts INTERFACE "/Zo")
        else(MSVC)
            target_compile_options(btree_compile_opts INTERFACE "-Og")
        endif()
    endif()
else()
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        message(
            WARNING
                "Optimization is on for all types but Debug, for obvious reasons."
        )
        message(
            WARNING "Turning off optimization is only allowed in debug build.")
    else()
        if(MSVC)
            target_compile_options(btree_compile_opts INTERFACE "/Od")
        else(MSVC)
            target_compile_options(btree_compile_opts INTERFACE "-O0")
        endif()
    endif()
endif()

if(btree_USE_LLD)
    find_program(LLD lld REQUIRED)
    if(NOT LLD)
        message(
            FATAL_ERROR
                "Error, LLD not found, please either turn off option USE_LLD or add directory of LLD to PATH"
        )
    endif()
    set(CMAKE_LINKER lld)
endif()

if(btree_USE_LIBCXX)
    if(NOT MSVC)
        target_compile_options(btree_compile_opts INTERFACE "-stdlib=libc++")
        target_link_options(btree_compile_opts INTERFACE
                            "-stdlib=libc++;-lc++abi")
    endif()
endif()

if(btree_ENABLE_TESTING)
    add_subdirectory(unittest)
endif()
if(btree_ENABLE_WARNING)
    if(MSVC)
        target_compile_options(btree_compile_opts INTERFACE "/W4")
    else(MSVC)
        target_compile_options(
            btree_compile_opts
            INTERFACE
                "-Wall;-Wextra;-Wformat=2;-fdiagnostics-color=always;-Wshadow;-Wconversion"
        )
    endif()
endif()

if(btree_WARNING_AS_ERROR)
    if(MSVC)
        target_compile_options(btree_compile_opts INTERFACE "/WX")
    else(MSVC)
        target_compile_options(btree_compile_opts INTERFACE "-Werror")
    endif()
endif()

if(btree_ENABLE_ASAN)
    if(MSVC)
        target_compile_options(
            btree_compile_opts
            INTERFACE
                "/fsanitize=address;/D_DISABLE_VECTOR_ANNOTATION;/D_DISABLE_STRING_ANNOTATION"
        )
    else(MSVC)
        target_compile_options(
            btree_compile_opts
            INTERFACE "-fsanitize=address;-fno-omit-frame-pointer")
        target_link_options(btree_compile_opts INTERFACE "-fsanitize=address")
    endif()
endif()

if(btree_ENABLE_UBSAN)
    if(MSVC)
        message(
            "We don't know if there's UBSan support on MSVC :(. Currently disabling it."
        )
    else(MSVC)
        target_compile_options(btree_compile_opts
                               INTERFACE "-fsanitize=undefined")
        target_link_options(btree_compile_opts INTERFACE "-fsanitize=undefined")
    endif()
endif()

if(btree_ENABLE_MSAN)
    if("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
        message(
            "We don't know if there's MSan support on MSVC :(. Currently disabling it."
        )
    else()
        target_compile_options(
            btree_compile_opts
            INTERFACE
                "-fsanitize=memory;-fno-omit-frame-pointer;-fno-optimize-sibling-calls"
        )
        target_link_options(btree_compile_opts INTERFACE "-fsanitize=memory")
    endif()
endif()

if(btree_ENABLE_COVERAGE)
    if(MSVC)
        target_compile_options("/fsanitize-coverage=edge")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(btree_compile_opts INTERFACE "--coverage")
        target_link_options(btree_compile_opts INTERFACE "--coverage")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL ".*Clang")
        target_compile_options(
            btree_compile_opts
            INTERFACE "-fprofile-instr-generate;-fcoverage-mapping")
        target_link_options(btree_compile_opts INTERFACE
                            "-fprofile-instr-generate;-fcoverage-mapping")
    endif()
endif()

if(btree_ENABLE_FUZZ)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(
            "Note that gcc/g++ won't work with libFuzzer. This is a LLVM-only tool."
        )
    else()
        add_subdirectory(fuzztest)
    endif()
endif()
