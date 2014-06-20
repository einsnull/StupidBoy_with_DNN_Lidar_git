#include "tcpServer.h"

int TCPServer::quote = 0;

TCPServer::TCPServer()
{
	if(quote == 0)
	{
		initSocket();
	}
	quote++;
}

TCPServer::TCPServer(int port)
{
	if(quote == 0)
	{
		initSocket();
	}
	listenConnection(port);
	quote++;
}

TCPServer::~TCPServer()
{
	quote --;
	if(quote == 0)
	{
		WSACleanup();
	}
}

void TCPServer::initSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 1, 1 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return;
	}
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
        HIBYTE( wsaData.wVersion ) != 1 ) {
		WSACleanup( );
		return; 
	}
}

void TCPServer::listenConnection(int port)
{
	sockSrv=socket(AF_INET,SOCK_STREAM,0);	
	addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(port);
	bind(sockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	listen(sockSrv,5);
}

void TCPServer::acceptConnection()
{
	int len = sizeof(SOCKADDR);
	sockConn = accept(sockSrv,(SOCKADDR*)&addrClient,&len);
}

void TCPServer::closeSocket()
{
	closesocket(sockConn);
}

void TCPServer::sendData(char *buf,int len)
{
	int i = 0;
	int left = len;
	while(left > 1024)
	{
		send(sockConn,&buf[i],1024,0);
		i += 1024;
		left -= 1024;
		//printf("left %d\n",left);
	}
	if(left > 0)
	{
		//printf("i %d\n",i);
		send(sockConn,&buf[i],left,0);
	}
}

int TCPServer::recvData(char *buf,int len)
{
	return recv(sockConn,buf,len,0);
}
