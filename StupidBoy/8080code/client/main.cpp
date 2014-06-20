#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tcpClient.h"

//≤È’“¥Æ
int findStr(char *src,char *dst,int srcLen,int dstLen,int startPos)
{
    int i,j;
    for(i = startPos;i < srcLen - dstLen + 1;i++)
    {
        if(src[i] == dst[0])
        {
            if(dstLen == 1)
            {
                return i;
            }
            for(j = 1;j < dstLen;j++)
            {
                if(src[i + j] != dst[j])
                {
                    break;
                }
            }
            if(dst[j] == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

int getImage(char *buf,char *addr)//modifyed
{
    char tmpBuf[2048];
    TCPClient tcpClient;
    if(!tcpClient.connectHost(addr,8080))
    {
        return 0;
    }
    //send request
    tcpClient.sendData("GET /?action=snapshot HTTP/1.0\r\n\r\n",34);
    int len = tcpClient.recvData(tmpBuf,512);
    //find where image data starts
    int num = findStr(tmpBuf,"\r\n\r\n",512,4,255);
	//if error occurs,return -1
    if(num == -1)
    {
        tcpClient.disconnect();
        return -1;
    }
	
    //copy data to buf
	if(len - num - 4 > 0)
	{
		memcpy(buf,&tmpBuf[num+4],len - num - 4);
	}
    int pos = len - num - 4;
    while(1)
    {
        int n = tcpClient.recvData(&buf[pos],1024);
		if(n <= 0)
		{
			break;
		}
        pos += n;
    }
    tcpClient.disconnect();
    return pos;//return data length
}

int main()
{
	/*
	char buf[500*1024];
	TCPClient cln;
	cln.connectHost("127.0.0.1",8080);
	FILE *fp = fopen("mytest.jpg","wb");
	cln.sendData("GET /?action=snapshot HTTP/1.0\r\n\r\n",34);
	int l = cln.recvData(buf,263);
	buf[l] = NULL;
	int num = findStr(buf,"\r\n\r\n",512,4,255);
	printf("num %d\n",num);
	printf("l : %d %s\n",l,buf);
	if(fp)
	{
		while(1)
		{
			int n = cln.recvData(buf,1024);
			printf("n:%d\n",n);
			if(n <= 0)
			{
				break;
			}
			fwrite(buf,sizeof(char),n,fp);
		}
		cln.disconnect();
		fclose(fp);
	}*/
	
	char buf[500*1024];
	int len  = getImage(buf,"127.0.0.1");
	FILE *fp = fopen("mytest.jpg","wb");
	if(fp)
	{	
		fwrite(buf,sizeof(char),len,fp);
		fclose(fp);
	}
	printf("len: %d\n",len);
	
	return 0;
}