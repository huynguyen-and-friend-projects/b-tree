add_library(b-tree STATIC b-tree.cxx)
target_link_libraries(b-tree PRIVATE compile-opts)
target_include_directories(b-tree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
