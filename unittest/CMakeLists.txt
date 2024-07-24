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

add_executable(b-tree-unit-test test-b-tree.cxx)
target_link_libraries(b-tree-unit-test PRIVATE b-tree b-tree-compile-opts GTest::gtest_main)
gtest_discover_tests(b-tree-unit-test)
