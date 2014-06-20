#include "tcpclient.h"

int TCPClient::quote = 0;//���ü���

//���캯��
TCPClient::TCPClient() 
{
	if(quote == 0)//���ü���Ϊ0ʱ������winsock dll
	{
		initSocket();
	}
	quote++;
}

//��������
TCPClient::~TCPClient()
{
	quote--;
	if(quote == 0)//���ü���Ϊ0ʱ��ж��winsock dll
	{
		WSACleanup();
	}
}

void TCPClient::disconnect()//�Ͽ�����
{
	closesocket(sockClient);
}

int TCPClient::recvData(char *buf,int bufSize)//��������
{
	return recv(sockClient,buf,bufSize,0);
}

//����
bool TCPClient::connectHost(char *addr,int port)
{
	addrSrv.sin_addr.S_un.S_addr=inet_addr(addr);
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(port);
	sockClient = socket(AF_INET,SOCK_STREAM,0);
	int result = connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	if(result == SOCKET_ERROR)
	{
		closesocket(sockClient);
		return false;
	}
	/*unsigned long ul = 1;
    ioctlsocket(sockClient, FIONBIO, &ul);
	connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	struct timeval timeout;
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(sockClient,&fs);
	timeout.tv_sec = 0;
	timeout.tv_usec = 800;
	int ret = select(0, 0, &fs, 0, &timeout);
	if(ret <= 0)
	{
		closesocket(sockClient);
		return false;
	}
	else
	{
		ul = 0;
		ret = ioctlsocket(sockClient, FIONBIO, &ul);
		if(ret == SOCKET_ERROR)
		{
			closesocket (sockClient);
			return false;
		}
	}*/
	return true;
}


//����
bool TCPClient::connectHost2(char *addr,int port)
{
	addrSrv.sin_addr.S_un.S_addr=inet_addr(addr);
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(port);
	sockClient = socket(AF_INET,SOCK_STREAM,0);
	unsigned long ul = 1;
    ioctlsocket(sockClient, FIONBIO, &ul);
	connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	struct timeval timeout;
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(sockClient,&fs);
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;
	int ret = select(0, 0, &fs, 0, &timeout);
	if(ret <= 0)
	{
		closesocket(sockClient);
		return false;
	}
	else
	{
		ul = 0;
		ret = ioctlsocket(sockClient, FIONBIO, &ul);
		if(ret == SOCKET_ERROR)
		{
			closesocket (sockClient);
			return false;
		}
	}
	return true;
}

//��������
void TCPClient::sendData(char *buf,int len)
{
	int i = 0;
	int left = len;
	while(left > 1024)
	{
		send(sockClient,&buf[i],1024,0);
		i += 1024;
		left -= 1024;
		//printf("left %d\n",left);
	}
	if(left > 0)
	{
		//printf("i %d\n",i);
		send(sockClient,&buf[i],left,0);
	}
}

//����WinSock DLL
void TCPClient::initSocket()
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
