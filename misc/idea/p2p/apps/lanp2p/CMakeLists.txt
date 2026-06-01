add_executable(lanp2p
  src/main.cpp
  src/cli/args.cpp
  src/cli/commands.cpp
)

target_include_directories(lanp2p PRIVATE
  include
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lanp2p PRIVATE
  lanp2p_core
  lanp2p_transfer
  lanp2p_index
  lanp2p_common
  Threads::Threads
)

lanp2p_apply_defaults(lanp2p)
