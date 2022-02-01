#include<iostream>
#include "async_log.h"
#include "inet_address.h"
#include "event_loop.h"
#include "tcp_server.h"

using namespace net;

#ifdef WIN32
//³õÊ¼»¯Windows socket¿â
NetworkInitializer windowsNetworkInitializer;
#endif

void messageCb(const TcpConnectionPtr& connection, Buffer* buffer, Timestamp timestamp)
{
	std::string msg = buffer->retrieveAllAsString();
	std::cout << "msg: " << msg << std::endl;

	connection->send(msg);
}

int main(int argc, char** argv)
{
#ifdef _DEBUG
	CAsyncLog::init("tcpServerTest");
	CAsyncLog::setLevel(LOG_LEVEL_DEBUG);
#else
	CAsyncLog::init("tcpServerTest");
	CAsyncLog::setLevel(LOG_LEVEL_DEBUG);
#endif

	EventLoop loop;
	InetAddress serverAddr(8888);
	TcpServer serverInst(&loop, serverAddr, "myServer");
	serverInst.setMessageCallback(messageCb);
	serverInst.start(1);
	loop.loop();

	getchar();
	
	return 0;
}