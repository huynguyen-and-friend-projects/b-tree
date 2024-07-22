add_executable(fuzzing fuzz-test.cxx)
target_compile_options(fuzzing PRIVATE "-fsanitize=fuzzer")
target_link_libraries(fuzzing PRIVATE b-tree b-tree-compile-opts -fsanitize=fuzzer)
