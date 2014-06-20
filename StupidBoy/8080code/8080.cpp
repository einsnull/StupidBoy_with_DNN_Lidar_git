#include <stdio.h>
#include <string.h>
#include "tcpServer.h"

int loadFile(char *szFileName,char *buf);

int main()
{
	int numOfExamples = 9;
	FILE *fp = fopen("numOfExamples.txt","r");
	if(fp!=NULL)
	{
		fscanf(fp,"%d",&numOfExamples);
		printf("Num of Examples: %d\n",numOfExamples);
		fclose(fp);
	}
	else
	{
		printf("Couldn't open \"numOfExamples.txt\"");
		return -1;
	}
	char str[] = "HTTP/1.0 200 OK\r\nConnection: close\r\nServer: MJPG-Streamer/0.2\r\nCache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\nPragma: no-cache\r\nExpires: Mon, 3 Jan 2000 12:34:56 GMT\r\nContent-type: image/jpeg\r\nX-Timestamp: 167.413541\r\n\r\n";
	TCPServer srv(8080);
	char **buf = new char*[numOfExamples];
	int i;
	for(i = 0;i < numOfExamples;i++)
	{
		buf[i] = new char[500*1024];
	}
	char tmpBuf[4*1024];
	int size[9];
	for(i = 0; i < numOfExamples; i++)
	{
		char szFileName[255];
		sprintf(szFileName,"%d.jpg",i);
		size[i] = loadFile(szFileName,buf[i]);
	}
	printf("File Loaded.\n");
	while(1)
	{
		for(i = 0;i < numOfExamples; i++)
		{
			srv.acceptConnection();
			int len = srv.recvData(tmpBuf,1024);
			tmpBuf[len] = NULL;
			printf("%d %s\n",len,tmpBuf);
			//srv.sendData("Hello?",6);
			srv.sendData(str,strlen(str));
			srv.sendData(buf[i],size[i]);		
			srv.closeSocket();
			printf("size:%d\n",size[i]);
		}
	}
	for(i = 0;i < numOfExamples;i++)
	{
		delete []buf[i];
	}
	delete []buf;
	return 0;
}

int loadFile(char *szFileName,char *buf)
{
	FILE *fp = fopen(szFileName,"rb");
	char tmpBuf[4*1024];
	int size = 0;
	if(fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		int n;
		int index = 0;
		int left = size;
		while(left > 0)
		{
			n = fread(tmpBuf,sizeof(char),4*1024,fp);
			memcpy(&buf[index],tmpBuf,n);
			index += n;
			left -= n;
			//printf("n:%d left:%d\n",n,left);
		}
		fclose(fp);
	}
	return size;
}