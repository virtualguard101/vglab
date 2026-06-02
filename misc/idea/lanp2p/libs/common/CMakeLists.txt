add_library(lanp2p_common STATIC
  src/identity.cpp
)

target_include_directories(lanp2p_common
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

lanp2p_apply_defaults(lanp2p_common)
