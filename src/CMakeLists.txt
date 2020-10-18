project(network)

cmake_minimum_required(VERSION 2.6)
  
add_library(net SHARED
fixed_byte_buffer.cpp
core_dump.cpp
file.cpp
thread.cpp
count_down_latch.cpp
log.cpp
time.cpp
inet_address.cpp
socket.cpp
channel.cpp
epoll.cpp
event_loop.cpp
acceptor.cpp
event_loop_thread.cpp
)

target_link_libraries(net pthread systemd)

add_executable(network main.cpp)
 
target_link_libraries(network net pthread)
 