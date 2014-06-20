#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

class TCPServer
{
private:
	static int quote;
	SOCKET sockSrv;
	SOCKET sockConn;
	SOCKADDR_IN addrSrv;
	SOCKADDR_IN addrClient;
	void initSocket();
public:
	void acceptConnection();
	void sendData(char *buf,int len);
	int recvData(char *buf,int len);
	void listenConnection(int port);
	void closeSocket();
	TCPServer(int port);
	TCPServer();
	~TCPServer();
};