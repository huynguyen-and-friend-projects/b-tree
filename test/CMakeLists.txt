enable_testing()
find_package(GTest)
if(NOT GTest_DIR)
    message("GTest NOT found. Proceed to installation")
    # download the dependency
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker
    # settings
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

add_executable(test-exe test-b-tree.cxx)
target_link_libraries(test-exe PRIVATE b-tree compile-opts GTest::gtest_main)
if(ENABLE_PCH)
    target_precompile_headers(test-exe PRIVATE <gtest/gtest.h>)
endif()
if(ENABLE_ASAN)
    # I think this is discouraged by the devs, but it wouldn't run without this
    # option.
    target_link_libraries(test-exe PRIVATE asan)
endif()
gtest_discover_tests(test-exe)
# add_custom_command(
#     TARGET test-exe
#     POST_BUILD
#     COMMAND ${CMAKE_CTEST_COMMAND}
#     USES_TERMINAL
#     COMMENT "Run tests after compilation")
