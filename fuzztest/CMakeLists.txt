enable_testing()

add_executable(b-tree-fuzz-test fuzz-test.cxx)
target_compile_options(b-tree-fuzz-test PRIVATE "-fsanitize=fuzzer")
target_link_libraries(b-tree-fuzz-test PRIVATE b-tree b-tree-compile-opts)
target_link_options(b-tree-fuzz-test PRIVATE -fsanitize=fuzzer)
# not recommended to use unless you want to run some automated testing
add_test(
    NAME run-fuzz-test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND $<TARGET_FILE:b-tree-fuzz-test> -max_total_time=10 COMMAND_EXPAND_LISTS)
