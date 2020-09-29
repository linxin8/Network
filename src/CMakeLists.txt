project(network)

cmake_minimum_required(VERSION 2.6)
  
add_library(net SHARED
fixed_byte_buffer.cpp
core_dump.cpp
file.cpp
)
add_executable(network main.cpp)
 
target_link_libraries(network net pthread)
 