project(network)

cmake_minimum_required(VERSION 2.6)
  
add_library(net SHARED
fixed_byte_buffer.cpp
core_dump.cpp
file.cpp
thread.cpp
count_down_latch.cpp
log.cpp
)
add_executable(network main.cpp)
 
target_link_libraries(network net pthread)
 