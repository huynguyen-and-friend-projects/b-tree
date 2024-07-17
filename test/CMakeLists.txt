enable_testing()
find_package(GTest)
if(NOT GTest_DIR)
    message("GTest NOT found. Proceed to installation")
    # download the dependency
    include(FetchContent)
    FetchContent_Declare(
        GTEST
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG f8d7d77) # v1.14.0
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(GTEST)
endif()

include(GoogleTest)

add_executable(test-exe test-b-tree.cxx)
target_link_libraries(test-exe PRIVATE b-tree compile-opts GTest::gtest_main)
if(ENABLE_ASAN)
    # I think this is discouraged by the devs, but it wouldn't run without this
    # option.
    target_link_libraries(test-exe PRIVATE asan)
endif()
gtest_discover_tests(test-exe)
add_custom_target(
    test-all
    COMMAND cmake --build ${PROJECT_BINARY_DIR} && ctest
            ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Run all the tests"
    USES_TERMINAL)
