add_library(b-tree INTERFACE)
target_include_directories(b-tree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if(btree_CLANGD_COMPAT)
    target_link_libraries(b-tree INTERFACE btree_compile_opts)
else()
    target_compile_features(b-tree INTERFACE cxx_std_20)
endif()

if(ENABLE_PCH)
    target_precompile_headers(
        b-tree
        INTERFACE
        b-tree.hxx
        <array>
        <cassert>
        <climits>
        <concepts>
        <cstddef>
        <memory>
        <optional>
        <stdexcept>
        <type_traits>
        <utility>)
endif()
