#include<iostream>
#include "async_log.h"
#include "inet_address.h"
#include "event_loop.h"
#include "tcp_client.h"
#include <thread>

using namespace net;

#ifdef WIN32
//³õÊ¼»¯Windows socket¿â
NetworkInitializer windowsNetworkInitializer;
#endif

void connectionCb(const TcpConnectionPtr& connection) {
	connection->send("hello world");
}

void messageCb(const TcpConnectionPtr& connection, Buffer* buffer, Timestamp timestamp) {
	std::string msg = buffer->retrieveAllAsString();
	std::cout << "receive message: " << msg << "\n";
	connection->send(msg);
}

int main(int argc, char** argv)
{
#ifdef _DEBUG
	CAsyncLog::init("tcpClientTest");
	CAsyncLog::setLevel(LOG_LEVEL_ERROR);
#else
	CAsyncLog::init("tcpClientTest");
	CAsyncLog::setLevel(LOG_LEVEL_DEBUG);
#endif

	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 8888);
	TcpClient client(&loop, serverAddr, "clent");
	client.setConnectionCallback(connectionCb);
	client.setMessageCallback(messageCb);
	client.connect();
	loop.loop();

	getchar();
	
	return 0;
}