cmake_minimum_required(VERSION 3.20)
project(netstack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(netstack_core
  src/netstack/address.cc
  src/netstack/subnet.cc
)
target_include_directories(netstack_core PUBLIC include)

add_library(netstack_ports
  src/ports/port_manager.cc
)
target_link_libraries(netstack_ports PUBLIC netstack_core)
target_include_directories(netstack_ports PUBLIC include)

add_library(netstack_header
  src/header/checksum.cc
  src/header/ipv4.cc
  src/header/udp.cc
  src/header/tcp.cc
)
target_link_libraries(netstack_header PUBLIC netstack_core)
target_include_directories(netstack_header PUBLIC include)

add_library(netstack_stack
  src/stack/network_protocol.cc
  src/stack/nic.cc
  src/stack/stack.cc
  src/stack/transport_demuxer.cc
  src/net/ipv4/protocol.cc
  src/net/ipv4/send.cc
)
target_link_libraries(netstack_stack PUBLIC netstack_header netstack_ports)
target_include_directories(netstack_stack PUBLIC include)

add_library(netstack_transport_udp
  src/transport/udp/protocol.cc
  src/transport/udp/endpoint.cc
)
target_link_libraries(netstack_transport_udp PUBLIC netstack_stack)
target_include_directories(netstack_transport_udp PUBLIC include)

add_library(netstack_transport_tcp
  src/transport/tcp/protocol.cc
  src/transport/tcp/connection.cc
  src/transport/tcp/listener.cc
)
target_link_libraries(netstack_transport_tcp PUBLIC netstack_stack)
target_include_directories(netstack_transport_tcp PUBLIC include)

add_library(netstack_link
  src/link/loopback.cc
  src/link/channel.cc
)
target_link_libraries(netstack_link PUBLIC netstack_stack)
target_include_directories(netstack_link PUBLIC include)

option(NETSTACK_ENABLE_TUN "Build TUN/fdbased link and examples" OFF)
option(NETSTACK_TUN_INTEGRATION_TEST
       "Add ctest for fdbased socketpair unit test (requires NETSTACK_ENABLE_TUN)" OFF)

if(NETSTACK_ENABLE_TUN)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "NETSTACK_ENABLE_TUN requires Linux")
  endif()
  target_sources(netstack_link PRIVATE
    src/link/tun.cc
    src/link/fdbased.cc
  )
endif()

add_executable(netstack_header_test tests/header/ipv4_test.cc)
target_link_libraries(netstack_header_test PRIVATE netstack_header)

add_executable(netstack_header_udp_test tests/header/udp_test.cc)
target_link_libraries(netstack_header_udp_test PRIVATE netstack_header)

add_executable(netstack_header_tcp_test tests/header/tcp_test.cc)
target_link_libraries(netstack_header_tcp_test PRIVATE netstack_header)

add_executable(netstack_seqnum_test tests/seqnum/seqnum_test.cc)
target_link_libraries(netstack_seqnum_test PRIVATE netstack_core)

add_executable(netstack_stack_test tests/stack/loopback_dispatch_test.cc)
target_link_libraries(netstack_stack_test PRIVATE netstack_link)

add_executable(netstack_m0_test tests/m0/ipv4_acceptance_test.cc)
target_link_libraries(netstack_m0_test PRIVATE netstack_link)

add_executable(netstack_m1_test tests/m1/udp_echo_test.cc)
target_link_libraries(netstack_m1_test PRIVATE netstack_link netstack_transport_udp)

add_executable(netstack_m2_tcp_handshake_test tests/m2/tcp_handshake_test.cc)
target_link_libraries(netstack_m2_tcp_handshake_test PRIVATE netstack_link netstack_transport_tcp)
target_include_directories(netstack_m2_tcp_handshake_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(netstack_m2_tcp_transfer_test tests/m2/tcp_transfer_test.cc)
target_link_libraries(netstack_m2_tcp_transfer_test PRIVATE netstack_link netstack_transport_tcp)
target_include_directories(netstack_m2_tcp_transfer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(netstack_m2_tcp_connect_test tests/m2/tcp_connect_test.cc)
target_link_libraries(netstack_m2_tcp_connect_test PRIVATE netstack_link netstack_transport_tcp)
target_include_directories(netstack_m2_tcp_connect_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(netstack_m2_tcp_backlog_test tests/m2/tcp_backlog_test.cc)
target_link_libraries(netstack_m2_tcp_backlog_test PRIVATE netstack_link netstack_transport_tcp)
target_include_directories(netstack_m2_tcp_backlog_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME header_ipv4 COMMAND netstack_header_test)
add_test(NAME header_udp COMMAND netstack_header_udp_test)
add_test(NAME header_tcp COMMAND netstack_header_tcp_test)
add_test(NAME seqnum COMMAND netstack_seqnum_test)
add_test(NAME stack_loopback COMMAND netstack_stack_test)
add_test(NAME m0_ipv4_acceptance COMMAND netstack_m0_test)
add_test(NAME m1_udp_echo COMMAND netstack_m1_test)
add_test(NAME m2_tcp_handshake COMMAND netstack_m2_tcp_handshake_test)
add_test(NAME m2_tcp_transfer COMMAND netstack_m2_tcp_transfer_test)
add_test(NAME m2_tcp_connect COMMAND netstack_m2_tcp_connect_test)
add_test(NAME m2_tcp_backlog COMMAND netstack_m2_tcp_backlog_test)

if(NETSTACK_ENABLE_TUN)
  add_executable(tun_tcp_echo examples/tun_tcp_echo.cc)
  target_link_libraries(tun_tcp_echo PRIVATE netstack_link netstack_transport_tcp)

  if(NETSTACK_TUN_INTEGRATION_TEST)
    add_executable(netstack_fdbased_test tests/link/fdbased_test.cc)
    target_link_libraries(netstack_fdbased_test PRIVATE netstack_link)
    add_test(NAME link_fdbased COMMAND netstack_fdbased_test)
  endif()
endif()
