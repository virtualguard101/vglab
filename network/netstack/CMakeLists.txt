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

add_library(netstack_link
  src/link/loopback.cc
  src/link/channel.cc
)
target_link_libraries(netstack_link PUBLIC netstack_stack)
target_include_directories(netstack_link PUBLIC include)

add_executable(netstack_header_test tests/header/ipv4_test.cc)
target_link_libraries(netstack_header_test PRIVATE netstack_header)

add_executable(netstack_header_udp_test tests/header/udp_test.cc)
target_link_libraries(netstack_header_udp_test PRIVATE netstack_header)

add_executable(netstack_stack_test tests/stack/loopback_dispatch_test.cc)
target_link_libraries(netstack_stack_test PRIVATE netstack_link)

add_executable(netstack_m0_test tests/m0/ipv4_acceptance_test.cc)
target_link_libraries(netstack_m0_test PRIVATE netstack_link)

add_executable(netstack_m1_test tests/m1/udp_echo_test.cc)
target_link_libraries(netstack_m1_test PRIVATE netstack_link netstack_transport_udp)

enable_testing()
add_test(NAME header_ipv4 COMMAND netstack_header_test)
add_test(NAME header_udp COMMAND netstack_header_udp_test)
add_test(NAME stack_loopback COMMAND netstack_stack_test)
add_test(NAME m0_ipv4_acceptance COMMAND netstack_m0_test)
add_test(NAME m1_udp_echo COMMAND netstack_m1_test)
