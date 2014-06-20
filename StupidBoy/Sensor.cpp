#include "stdafx.h"
#include "Sensor.h"
#include "myString.h"
#include "FreeImage.h"
#include "ImageFunctions.h"


#define SENSOR_HEAD_MIN_DIST 100
#define SENSOR_OK_MIN_DIST 40
#define SENSOR_SIDE_MIN_DIST 30
#define HEAD_RANGE 40
#define HEAD_START (270 - HEAD_RANGE/2)
#define FRONT_SIDE_RANGE 20
#define FRONT_LEFT_START (HEAD_START)
#define FRONT_RIGHT_START ((HEAD_START + HEAD_RANGE) % 360)
#define LEFT_START (FRONT_LEFT_START - FRONT_SIDE_RANGE)
#define RIGHT_START ((FRONT_RIGHT_START + FRONT_SIDE_RANGE) % 360)
#define SIDE_RANGE 80
#define FRONT_SIDE_OK_RANGE 60
#define FRONT_LEFT_OK_START (HEAD_START)
#define FRONT_RIGHT_OK_START ((HEAD_START + HEAD_RANGE) % 360)
#define LEFT_OK_START (FRONT_LEFT_OK_START - FRONT_SIDE_OK_RANGE)
#define RIGHT_OK_START ((FRONT_RIGHT_OK_START + FRONT_SIDE_OK_RANGE) % 360)
#define SIDE_OK_RANGE 40


bool getSensorData(int *dataArr,char *addr)
{
	TCPClient client;
	int port = 2015;
	
	if(client.connectHost(addr,port))
	{
		unsigned char buf[361] = {0};
		int n = client.recvData((char *)buf,360);
		buf[n] = 0;
		addLog(n);
		client.disconnect();
		for(int i = 0; i < n; i++)
		{
			//dataArr[i] = 0;
			dataArr[i] = (int)buf[i];
		}
		return true;
	}
	return false;
}

bool getFrontState(int *data)
{
	//return data[0] > SENSOR_OK_MIN_DIST || data[0] == 0;
	for(int i = HEAD_START; i < HEAD_START + HEAD_RANGE;i++)
	{
		if(data[i % 360] < SENSOR_OK_MIN_DIST && data[i % 360] != 0) 
		{
			return false;
		}
	}
	return true;
}

//获取距离传感器数据
bool getFrontObstacle(int *data)
{
	for(int i = HEAD_START; i < HEAD_START + HEAD_RANGE;i++)
	{
		if(data[i % 360] < SENSOR_HEAD_MIN_DIST && data[i % 360] != 0) 
		{
			return true;
		}
	}
	return false;
}

bool getLeftObstacle(int *data)
{
	for(int i = LEFT_START; i > LEFT_START - SIDE_RANGE; i--)
	{
		if(data[i % 360] < SENSOR_SIDE_MIN_DIST && data[i % 360] != 0)
		{
			return true;
		}
	}
	return false;
}

bool getRightObstacle(int *data)
{
	for(int i = RIGHT_START; i < RIGHT_START + SIDE_RANGE; i++)
	{
		if(data[i % 360] < SENSOR_SIDE_MIN_DIST && data[i % 360] != 0)
		{
			return true;
		}
	}
	return false;
}

bool getLeftState(int *data)
{
	for(int i = LEFT_OK_START; i > LEFT_OK_START - SIDE_OK_RANGE; i--)
	{
		if(data[i % 360] < SENSOR_SIDE_MIN_DIST && data[i % 360] != 0)
		{
			return false;
		}
	}
	return true;
}

bool getRightState(int *data)
{
	for(int i = RIGHT_OK_START; i < RIGHT_OK_START + SIDE_OK_RANGE; i++)
	{
		if(data[i % 360] < SENSOR_SIDE_MIN_DIST && data[i % 360] != 0)
		{
			return false;
		}
	}
	return true;
}

bool getFrontLeftObstacle(int *data)
{
	for(int i = FRONT_LEFT_START; i > FRONT_LEFT_START - FRONT_SIDE_RANGE; i--)
	{
		if(data[i % 360] < SENSOR_HEAD_MIN_DIST && data[i % 360] != 0)
		{
			return true;
		}
	}
	return false;
}

bool getFrontRightObstacle(int *data)
{
	for(int i = FRONT_RIGHT_START; i < FRONT_RIGHT_START + FRONT_SIDE_RANGE; i++)
	{
		if(data[i % 360] < SENSOR_HEAD_MIN_DIST && data[i % 360] != 0)
		{
			return true;
		}
	}
	return false;
}

bool getFrontLeftState(int *data)
{
	for(int i = FRONT_LEFT_START; i > FRONT_LEFT_START - FRONT_SIDE_OK_RANGE; i--)
	{
		if(data[i % 360] < SENSOR_HEAD_MIN_DIST && data[i % 360] != 0)
		{
			return false;
		}
	}
	return true;
}

bool getFrontRightState(int *data)
{
	for(int i = FRONT_RIGHT_START; i < FRONT_RIGHT_START + FRONT_SIDE_OK_RANGE; i++)
	{
		if(data[i % 360] < SENSOR_HEAD_MIN_DIST && data[i % 360] != 0)
		{
			return false;
		}
	}
	return true;
}


//远程获取图片
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
        int n = tcpClient.recvData(&buf[pos],65536);
		if(n <= 0)
		{
			break;
		}
        pos += n;
    }
    tcpClient.disconnect();
    return pos;//return data length
}


//获取图片大小
int getRemoteImageSize(char *remoteAddr,int &width,int &height,int margin)
{
	int bufSize = 700*1024;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	int len = getImage((char*)jpgBuf,remoteAddr);//获取图片
	//打开内存流
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);
	CImage image;
	//未获取到图片，重新获取
	if(len = -1)
	{
		return -1;
	}
	decodeToCImage(hMem,image);//解码图片
	margin = abs(margin);
	width = image.GetWidth();
	height = image.GetHeight();
	int imgWidth = width-2*margin;
	FreeImage_CloseMemory(hMem);
	delete []jpgBuf;
	return imgWidth*height;
}

int testSensor(char *addr)
{
	int data[9] = {0};
	if(!getSensorData(data,addr))
	{
		return -1;
	}
	if(data[0] != 0 && data[0] < SENSOR_HEAD_MIN_DIST)
	{
		return -1;
	}
	return 1;
}

inline int goLeft(int initState)
{
	return initState - 1;
}

inline int goRight(int initState)
{
	return initState;
}

inline int waitLeft(int initState)
{
	return -goLeft(initState) - 1;
}

inline int waitRight(int initState)
{
	return -goRight(initState) - 1;
}

//传感器策略
int sensorStrategy(CstupidBoyDlg *pStupidBoy,int curState,int initState)
{
	EnterCriticalSection(&(pStupidBoy->sensorCS));
	bool frontState = pStupidBoy->frontState;
	bool frontRightState = pStupidBoy->frontRightState;
	bool rightState = pStupidBoy->rightState;
	bool frontLeftState = pStupidBoy->frontLeftState;
	bool leftState = pStupidBoy->leftState;
	bool frontObstacle = pStupidBoy->frontObstacle;
	bool frontLeftObstacle = pStupidBoy->frontLeftObstacle;
	bool frontRightObstacle = pStupidBoy->frontRightObstacle;
	bool leftObstacle = pStupidBoy->leftObstacle;
	bool rightObstacle = pStupidBoy->rightObstacle;
	LeaveCriticalSection(&(pStupidBoy->sensorCS));
		
	if(goLeft(initState) && frontObstacle && frontLeftObstacle && frontRightObstacle)
	{
		return -3;
	}

	//如果当前正在靠左行驶，则判断是否能转换为靠右行驶，如果能，则转换
	if(curState == goLeft(initState) && frontRightState &&
		rightState && frontState)
	{
		return goRight(initState);
	}

	//test code
	//if(curState == goLeft(initState) && !frontRightObstacle && frontState && frontRightState && !rightState)
	//{
	//	return goRight(initState);
	//}
	
	//获取前方障碍物信息
	if(frontObstacle)
	{
		//如果前方有障碍物且可躲避且当前为靠右行驶,则转换为靠左行驶，其余情况等待
		if(frontState && curState == goRight(initState) && 
			!frontLeftObstacle && !leftObstacle)
		{
			return goLeft(initState);
		}
		else if(curState == goRight(initState))
		{
			return waitRight(initState);
		}
		else if(curState == goLeft(initState))
		{
			return waitLeft(initState);
		}
	}

	if(curState == waitLeft(initState))
	{
		if(frontState && !frontRightObstacle && !rightObstacle)
		{
			return goRight(initState);
		}
	}

	if(curState == waitRight(initState))
	{
		if(frontState)
		{
			return goRight(initState);
		}
	}

	/*if(rightObstacle)
	{
		if(curState == goRight(initState))
		{
			return goLeft(initState);
		}
	}*/

	return curState;
}