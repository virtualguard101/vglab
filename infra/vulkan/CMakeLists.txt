cmake_minimum_required(VERSION 3.16)

project(vulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

add_executable(
    get_arch src/get_arch.cpp
)
target_link_libraries(get_arch PRIVATE Vulkan::Vulkan)

target_compile_options(get_arch PRIVATE -Wall -Wextra -O2)
