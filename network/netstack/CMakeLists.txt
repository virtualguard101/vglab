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

add_executable(netstack_header_test tests/header/ipv4_test.cc)
target_link_libraries(netstack_header_test PRIVATE netstack_header)

enable_testing()
add_test(NAME header_ipv4 COMMAND netstack_header_test)
