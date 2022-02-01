#include<iostream>
#include "async_log.h"
#include "inet_address.h"
#include "event_loop.h"
#include "tcp_client.h"

using namespace net;

#ifdef WIN32
//��ʼ��Windows socket��
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
	CAsyncLog::init("tcpClientTest");
	CAsyncLog::setLevel(LOG_LEVEL_DEBUG);
#else
	CAsyncLog::init("tcpClientTest");
	CAsyncLog::setLevel(LOG_LEVEL_DEBUG);
#endif

	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 8888);
	TcpClient client(&loop, serverAddr, "clent");
	client.connect();
	loop.loop();

	getchar();
	
	return 0;
}