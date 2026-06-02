cmake_minimum_required(VERSION 3.16)
project(lanp2p VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# clangd / IDE: compile_commands.json in ${CMAKE_BINARY_DIR}
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Lanp2pHelpers)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)  # 使用 Boost 的 CONFIG 模式，不用旧 FindBoost
endif()
find_package(Boost 1.70 REQUIRED CONFIG)
# find_package(Boost 1.70 REQUIRED)

find_package(Threads REQUIRED)

add_subdirectory(libs)
add_subdirectory(apps)
