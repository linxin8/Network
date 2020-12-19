add_library(rpc SHARED
serialization.cpp
)
 
add_executable(server server.cpp)
target_link_libraries(server net rpc)

