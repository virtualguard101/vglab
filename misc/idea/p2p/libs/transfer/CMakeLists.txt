add_library(lanp2p_transfer STATIC
  src/tcp_channel.cpp
  src/server.cpp
  src/client.cpp
)

target_include_directories(lanp2p_transfer
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lanp2p_transfer
  PUBLIC
    lanp2p_protocol
    lanp2p_index
    lanp2p_common
)

lanp2p_link_boost(lanp2p_transfer)
lanp2p_apply_defaults(lanp2p_transfer)
