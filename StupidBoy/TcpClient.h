#pragma once
#include <Winsock2.h>
#include <stdio.h>
#pragma comment(lib,"Ws2_32.lib")


class TCPClient
{
public:
	static int quote;
private:
	SOCKADDR_IN addrSrv;
	SOCKET sockClient;
private:
	void initSocket();
public:
	TCPClient();
	void sendData(char *buf,int len);
	~TCPClient();
	int recvData(char *buf,int bufSize);
	bool connectHost(char *addr,int port);
	bool connectHost2(char *addr,int port);
	void disconnect();
};
