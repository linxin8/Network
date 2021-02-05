add_library(rpc SHARED
serialization.cpp
message.cpp
utility.cpp
rpc_server.cpp
rpc_client.cpp
rpc_register_server.cpp
)