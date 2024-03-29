#include <functional>

#include "tcp_client.h"
#include "async_log.h"
#include "platform.h"
#include "event_loop.h"
#include "socketsOps.h"
#include "connector.h"

namespace net
{
    namespace detail
    {

        void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
        {
            loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
        }

        void removeConnector(const ConnectorPtr& connector)
        {
            //connector->
        }

    }
}

BEGIN_NS(net)

TcpClient::TcpClient(EventLoop* loop,
    const InetAddress& serverAddr,
    const std::string& nameArg)
    : loop_(loop),
    connector_(std::make_shared<Connector>(loop, serverAddr)),
    name_(nameArg),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    retry_(false),
    connect_(true),
    nextConnId_(1)
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    LOGD("TcpClient::TcpClient[%s] - connector 0x%x", name_.c_str(), connector_.get());
}

TcpClient::~TcpClient()
{
    LOGD("TcpClient::~TcpClient[%s] - connector 0x%x", name_.c_str(), connector_.get());
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn)
    {
        if (loop_ != conn->getLoop())
            return;

        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique)
        {
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
    }
}

void TcpClient::connect()
{
    // FIXME: check state
    LOGD("TcpClient::connect[%s] - connecting to %s", name_.c_str(), connector_->serverAddress().toIpPort().c_str());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::sendMessage(const std::string& msg)
{
    if (connection_) {
        connection_->send(msg);
    }
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(SocketsOps::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(SocketsOps::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_ = conn;
    }

    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    if (loop_ != conn->getLoop())
        return;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (connection_ != conn)
            return;

        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_)
    {
        LOGD( "TcpClient::connect[%s] - Reconnecting to %s", name_.c_str(), connector_->serverAddress().toIpPort().c_str());
        connector_->restart();
    }
}

END_NS(net)