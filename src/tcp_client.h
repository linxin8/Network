#include "event_loop_thread.h"
#include "inet_address.h"
#include "socket.h"
#include "tcp_connection.h"
#include <functional>
#include <memory>

// all client share the same event loop thread
// all operations are thread safe with network thread
class TcpClient : Noncopyable
{
public:
    TcpClient();
    // return if connection is successful
    bool connectTo(InetAddress address);
    void setOnConnected(std::function<void()> onConnected);
    void setOnDisconnected(std::function<void()> onDisconnected);
    void setOnReadyToRead(std::function<void()> onReadyToRead);
    void setOnError(std::function<void()> onError);
    void disconnect();
    bool isConnected() const
    {
        return _isConnected;
    }

    void sendAsyn(std::string data);
    // read all data
    std::string read();
    // read all data and append to buffer
    void read(std::string& buffer);

    EventLoopThread* getNetworkThread()
    {
        return _thread;
    }

private:
    void onRead();
    void onDisconnected();
    void onConnected();
    void onError(int errorNo);

private:
    std::function<void()> _onConnected;
    std::function<void()> _onDisconnected;
    std::function<void()> _onReadyToRead;
    std::function<void()> _onError;
    std::function<void()> _onSend;
    std::string           _recvBuffer;
    std::string           _sendBuffer;
    EventLoopThread* _thread;  //  all client share the same event loop thread
    bool             _isConnected;
    std::shared_ptr<TcpConnection> _connection;
};