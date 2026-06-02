add_library(lanp2p_peer STATIC
  src/registry.cpp
)

target_include_directories(lanp2p_peer
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lanp2p_peer PUBLIC lanp2p_common)

lanp2p_apply_defaults(lanp2p_peer)
