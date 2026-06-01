add_library(lanp2p_discovery STATIC
  src/service.cpp
)

target_include_directories(lanp2p_discovery
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lanp2p_discovery
  PUBLIC
    lanp2p_protocol
    lanp2p_peer
)

lanp2p_link_boost(lanp2p_discovery)
lanp2p_apply_defaults(lanp2p_discovery)
