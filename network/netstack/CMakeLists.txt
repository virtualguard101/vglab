cmake_minimum_required(VERSION 3.20)
project(netstack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netstack_core
  src/netstack/address.cc
  src/netstack/subnet.cc
)
target_include_directories(netstack_core PUBLIC include)

add_library(netstack_header
  src/header/checksum.cc
  src/header/ipv4.cc
)
target_link_libraries(netstack_header PUBLIC netstack_core)
target_include_directories(netstack_header PUBLIC include)

add_library(netstack_stack
  src/stack/network_protocol.cc
  src/stack/nic.cc
  src/stack/stack.cc
  src/net/ipv4/protocol.cc
)
target_link_libraries(netstack_stack PUBLIC netstack_header)
target_include_directories(netstack_stack PUBLIC include)

add_library(netstack_link
  src/link/loopback.cc
  src/link/channel.cc
)
target_link_libraries(netstack_link PUBLIC netstack_stack)
target_include_directories(netstack_link PUBLIC include)

add_executable(netstack_header_test tests/header/ipv4_test.cc)
target_link_libraries(netstack_header_test PRIVATE netstack_header)

add_executable(netstack_stack_test tests/stack/loopback_dispatch_test.cc)
target_link_libraries(netstack_stack_test PRIVATE netstack_link)

add_executable(netstack_m0_test tests/m0/ipv4_acceptance_test.cc)
target_link_libraries(netstack_m0_test PRIVATE netstack_link)

enable_testing()
add_test(NAME header_ipv4 COMMAND netstack_header_test)
add_test(NAME stack_loopback COMMAND netstack_stack_test)
add_test(NAME m0_ipv4_acceptance COMMAND netstack_m0_test)
