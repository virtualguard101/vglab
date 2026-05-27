cmake_minimum_required(VERSION 3.20)

project(tcpip VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_library(${PROJECT_NAME}_lib STATIC
    ${CMAKE_SOURCE_DIR}/src/tcp.cc
    ${CMAKE_SOURCE_DIR}/src/ip.cc
    ${CMAKE_SOURCE_DIR}/src/utils.cc
)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
