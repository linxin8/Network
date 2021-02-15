find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(serialization serialization.cpp)
target_link_libraries(serialization ${GTEST_LIBRARIES} net rpc)
 
add_executable(message message.cpp)
target_link_libraries(message ${GTEST_LIBRARIES} net rpc)

add_executable(rpc_server rpc_server.cpp)
target_link_libraries(rpc_server net rpc)

add_executable(rpc_client rpc_client.cpp)
target_link_libraries(rpc_client net rpc)

add_executable(rpc_register_client rpc_register_client.cpp)
target_link_libraries(rpc_register_client net rpc)

add_executable(rpc_register_server rpc_register_server.cpp)
target_link_libraries(rpc_register_server net rpc)