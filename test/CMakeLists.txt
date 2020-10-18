find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}) 
 
add_executable(log log.cpp)
target_link_libraries(log ${GTEST_LIBRARIES} net)

add_executable(file file.cpp)
target_link_libraries(file ${GTEST_LIBRARIES} net)

add_executable(thread thread.cpp)
target_link_libraries(thread ${GTEST_LIBRARIES} net)

add_executable(circle_queue circle_queue.cpp)
target_link_libraries(circle_queue ${GTEST_LIBRARIES} net)

add_executable(inet_address inet_address.cpp)
target_link_libraries(inet_address ${GTEST_LIBRARIES} net)

add_executable(socket socket.cpp)
target_link_libraries(socket ${GTEST_LIBRARIES} net)

add_executable(event_loop event_loop.cpp)
target_link_libraries(event_loop ${GTEST_LIBRARIES} net)

add_executable(acceptor acceptor.cpp)
target_link_libraries(acceptor ${GTEST_LIBRARIES} net)

add_executable(tcp_buffer tcp_buffer.cpp)
target_link_libraries(tcp_buffer ${GTEST_LIBRARIES} net)