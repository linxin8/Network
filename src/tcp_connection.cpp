#include "tcp_connection.h"

void TcpConnection::send(const void* data, size_t size)
{
    _writeBuffer.append(data, size);
}