add_library(b-tree INTERFACE)
target_link_libraries(b-tree INTERFACE compile-opts)
target_include_directories(b-tree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_PCH)
    target_precompile_headers(
        b-tree
        INTERFACE
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
