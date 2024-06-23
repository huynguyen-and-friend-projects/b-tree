enable_testing()
find_library(GTEST GTest)
if(NOT GTEST)
    # download the dependency
    include(FetchContent)
    FetchContent_Declare(
        GTEST
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG f8d7d77) # v1.14.0
    FetchContent_MakeAvailable(GTEST)
endif()

add_executable(test-exe test-b-tree.cxx)
target_link_libraries(test-exe PRIVATE b-tree compile-opts GTest::gtest_main)
if(ENABLE_ASAN)
    # I think this is discouraged by the devs, but it wouldn't run without this
    # option.
    target_link_libraries(test-exe PRIVATE asan)
endif()
# pull the executable to project root
add_custom_command(
    TARGET test-exe
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:test-exe>"
            "${PROJECT_SOURCE_DIR}/$<TARGET_FILE_NAME:test-exe>"
    COMMENT "Copy the built executable to project root")
