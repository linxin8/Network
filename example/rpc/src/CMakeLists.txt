add_library(rpc SHARED
serialization.cpp
message.cpp
utility.cpp
)
 
add_executable(server server.cpp)
target_link_libraries(server net rpc)

add_executable(client client.cpp)
target_link_libraries(client net rpc)
