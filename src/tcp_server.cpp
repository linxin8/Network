#include "tcp_server.h"
#include <memory>

void defaultOnMessageReceived() {}
void defaultOnMessageSend() {}
void defaultOnConnection() {}

TcpServer::TcpServer() :
    _onConnection{defaultOnConnection},
    _onMessageReceived{defaultOnMessageReceived},
    _onMessageSend{defaultOnMessageSend}
{
    std::shared_ptr<int> p;
}