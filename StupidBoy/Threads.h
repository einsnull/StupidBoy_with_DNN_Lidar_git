#pragma once
#include <windows.h>
#include "stupidBoyDlg.h"
#include "ImageFunctions.h"
#include "DxInput.h"
#include "MatrixFunctions.h"
#include "GetConfig.h"
#include "StackedAE.h"

#include "Sensor.h"

#define GO_LEFT 0
#define GO_RIGHT 1

extern DWORD WINAPI viewThread(LPVOID lpParameter);
extern DWORD WINAPI collectDataThread(LPVOID lpParameter);
extern DWORD WINAPI viewSensorThread(LPVOID lpParameter);
extern DWORD WINAPI dxInputThread(LPVOID lpParameter);
extern DWORD WINAPI manualDriveThread(LPVOID lpParameter);
extern DWORD WINAPI autoDriveThread(LPVOID lpParameter);
extern DWORD WINAPI autoDriveThreadF(LPVOID lpParameter);
extern DWORD WINAPI manualDriveForNetView(LPVOID lpParameter);
extern bool drive(char speed,char direction,CstupidBoyDlg *pStupidBoy);

int getCurPos(MatrixXd &vector,Softmax &softmax)
{
	MatrixXd v = vector.transpose();
	int val = 0;
	MatrixXi pred = softmax.predict(v);
	val = pred(0,0);
	return val;
}

DWORD WINAPI selectModeThread(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bStopChildThread = true;

	while(!(pStupidBoy->bViewThreadRelease && pStupidBoy->bViewSensorThreadRelease
		&& pStupidBoy->bCollectDataThreadRelease && pStupidBoy->bManualDriveThreadRelease
		&& pStupidBoy->bAutoDriveThreadRelease && pStupidBoy->bAutoDriveThread_F_Release 
		&& pStupidBoy->bNetViewThreadRelease))
	{
		SwitchToThread();
	}

	pStupidBoy->bStopChildThread = false;

	switch(pStupidBoy->mode)
	{
	case 0:
		pStupidBoy->hChildThread = CreateThread(NULL,0,manualDriveThread,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 1:
		pStupidBoy->hChildThread = CreateThread(NULL,0,collectDataThread,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 2:
		pStupidBoy->hChildThread = CreateThread(NULL,0,autoDriveThread,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 3:
		pStupidBoy->hChildThread = CreateThread(NULL,0,viewSensorThread,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 4:
		pStupidBoy->bStopChildThread = true;
		pStupidBoy->hChildThread = NULL;
		break;
	case 5:
		pStupidBoy->hChildThread = CreateThread(NULL,0,viewThread,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 6:
		pStupidBoy->hChildThread = CreateThread(NULL,0,autoDriveThreadF,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	case 7:
		pStupidBoy->hChildThread = CreateThread(NULL,0,manualDriveForNetView,pStupidBoy,0,0);
		CloseHandle(pStupidBoy->hChildThread);
		break;
	}
	return 0;
}

DWORD WINAPI viewThread(LPVOID lpParameter)
{
	//参数获取
	//char *addr = "127.0.0.1"; //远程地址
	static int screenCaptureCnt = 0;
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;

	pStupidBoy->bViewThreadRelease = false;

	//内存分配，根据需要调整大小
	int bufSize = pStupidBoy->jpgBufSize;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	CreateDirectory(_T("ScreenCapture"),NULL);
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);
	pStupidBoy->addListStr(_T("ViewThread Enabled."));
	//线程主循环
	while( pStupidBoy->bStopChildThread != true)//stopDisplay?
	{
		CImage img;
		int len = getImage((char*)jpgBuf,pStupidBoy->remoteAddr);//获取图片
		//未获取到图片，重新获取
		if(len <= 0)
		{
			//pStupidBoy->addListStr(_T("img err"));
			continue;
		}
		//screen capture
		//解码图片

		//grayMode ?
		if(pStupidBoy->bGrayMode == true)
		{
			decodeToGrayCImage(hMem,img);//getGrayImage
		}
		else
		{
			decodeToCImage(hMem,img);//getImage
		}
		display(pStupidBoy->hStaticViewDC,img,pStupidBoy->displayRatio);//display image
		//screen capture
		if(pStupidBoy->bScreenCapture == true)
		{
			pStupidBoy->bScreenCapture = false;
			TCHAR strFileName[255];
			_stprintf_s(strFileName,_T("ScreenCapture\\%d.jpg"),screenCaptureCnt++);
			img.Save(strFileName);
		}
		img.Destroy();
		//Sleep(20);
	}
	FreeImage_CloseMemory(hMem);
	delete []jpgBuf;
	pStupidBoy->addListStr(_T("ViewThread Exit."));
	pStupidBoy->bViewThreadRelease = true;
	return 0;
}

int getSpeed(int dxSpeed,int speedThreshold)
{
	/*int id;
	int table[13] = {140,145,150,155,160,165,170,175,180,185,190,195,200};
	*/
	int speedRange = 60;
	int speed = 0;
	int bias = 0; // 120
	if(dxSpeed >= 65535/2)
	{
		speed = (int)((double)(dxSpeed - 65535/2)/(65535/2)*speedRange)/5*5 + bias;
	}
	else
	{
		speed = (int)((double)(dxSpeed - 65535/2)/(65535/2)*speedRange)/5*5 - bias;
	}
	if(abs(speed) < speedThreshold)
	{
		speed = 0;
	}
	return -speed;
}

int getLabel(int direction,int dirNum,int tableMinVal,int tableMaxVal)
{
	int bias = 1200;
	int val = (int)(((double)(direction + bias)/(double)65535)*180);
	//val -= 90;
	//val *= 0.5;
	//val += 90;
	//tableMinVal = 50;
	//tableMaxVal = 130;
	if(val < tableMinVal)
	{
		val = tableMinVal;
	}
	if(val > tableMaxVal)
	{
		val = tableMaxVal;
	}
	
	int margin = tableMaxVal - tableMinVal;
	if (margin <= 0)
	{
		AfxMessageBox(_T("Error margin val"));
		return 0;
	}
	val -= tableMinVal;
	int label = (int)(val/((double)margin/dirNum));
	if(label >= dirNum)
	{
		label = dirNum-1;
	}
	return label; // return value is in range 0 ~ 12
	/*int val = 0;
	 0 ~ 1  0.1 0.2 0.3 ... 0.9
	int margin = (int)(65535.0 / dirNum);
	val = (int)((double)(direction+1) / margin);
	if(val == dirNum)
	{
		val = dirNum - 1;
	}
	val = val + 1;
	return val;*/
}

int getDirection(int label,int dirNum,int* dirTable)
{
	int val = 0;
	//int table[DIR_NUM] = {60,65,70,75,80,85,90,95,100,105,110,115,120};
	//int table[DIR_NUM] = {60,70,80,90,100,110,120};
	if(label < 0)
	{
		label = 0;
	}
	if(label > dirNum-1)
	{
		label = dirNum-1;
	}
	val = dirTable[label];
	return val;
}

DWORD WINAPI dxInputThread(LPVOID lpParameter)
{
	//获取参数
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bDxInputThreadRelease = false;
	HWND hWnd = pStupidBoy->hMainDlg;
	//HWND hSlider = pStupidBoy->hDxInputSlider;
	HINSTANCE hInstance = pStupidBoy->hInstance;
	//初始化dxInput
	DXINPUT dxInput(hWnd,hInstance,DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
	//提示信息
	pStupidBoy->addListStr(_T("dxInput initialized"));

	int outputLayerSize = pStupidBoy->outputLayerSize;
	int speedThreshold = pStupidBoy->speedThreshold;
	int *dirTable = pStupidBoy->dirTable;

	//线程主循环
	while(pStupidBoy->bStopDxInput != true)
	{
		//更新控制信息
		CTRLS ctrls = dxInput.dxInputUpdate();
		//设置驾驶速度模式
		if(ctrls.keyId == 1)
		{
			if(pStupidBoy->bStepDriveMode == false)
			{
				pStupidBoy->bStepDriveMode = true;
				pStupidBoy->addListStr(_T("Step Drive Mode Enabled."));
			}
		}
		if(ctrls.keyId == 3)
		{
			if(pStupidBoy->bStepDriveMode == true)
			{
				pStupidBoy->bStepDriveMode = false;
				pStupidBoy->addListStr(_T("Step Drive Mode Disabled."));
			}
		}
		if(ctrls.keyId == 5)
		{
			ctrls.x = 65535/2;
		}
		if(ctrls.keyId == 4)
		{
			if(pStupidBoy->bTurningSupply == false)
			{
				pStupidBoy->bTurningSupply = true;
				pStupidBoy->addListStr(_T("Turning Supply Enabled."));
			}
		}
		if(ctrls.keyId == 6)
		{
			
			if(pStupidBoy->bTurningSupply == true)
			{
				pStupidBoy->bTurningSupply = false;
				pStupidBoy->addListStr(_T("Turning Supply Disabled."));
			}
		}
		//互斥更新方向
		pStupidBoy->setDxInputDirection(ctrls.x);
		//更新视窗滑动条位置
		
		int label = getLabel(ctrls.x,outputLayerSize,dirTable[0],dirTable[outputLayerSize-1]);
		int speed = getSpeed(ctrls.y,speedThreshold);
		//更新速度和方向值到窗口
		pStupidBoy->setDxInputStatic(label,speed);
		//用滑动条显示速度和方向值
		pStupidBoy->setDxInputSilder(label);
		pStupidBoy->setDxSpeedSlider(200 - speed);
		
		Sleep(20);
	}
	pStupidBoy->addListStr(_T("dxInputThread Exit."));
	pStupidBoy->bDxInputThreadRelease = true;
	return 0;
}


bool drive(int speed,int direction,CstupidBoyDlg *pStupidBoy)
{
	//记录上一条指令值
	//static int oldSpeed = 0;
	//static int oldDirection = 0;

	//
	////判断方向是否改变
	//if(direction == oldDirection && speed == oldSpeed)
	//{
	//	return false;
	//}

	////更新记录
	//oldSpeed = speed;
	//oldDirection = direction;
	

	//char *addr = "127.0.0.1"; //远程地址
	int port = 2013;//端口
	char cmdStr[100] = {0};
	//格式化字符串数据
	
	sprintf_s(cmdStr,"%d %d",speed,direction);
	/*FILE *fp;
	fopen_s(&fp,"test.txt","a+");
	if(fp != NULL )
	{
		fprintf(fp,"%d %d\n",(unsigned char)speed,direction);
		fclose(fp);
	}*/
	//发送数据
	if(pStupidBoy->tcpDriveClient.connectHost(pStupidBoy->remoteAddr,port)) //连接
	{
		//TCHAR msg[200] = _T("Sending data");
		//pStupidBoy->addListStr(msg);
		pStupidBoy->tcpDriveClient.sendData(cmdStr,(int)strlen(cmdStr)); //发送
		pStupidBoy->tcpDriveClient.disconnect(); //断开连接
		return true;
	}
	// error info


	TCHAR str[200] = _T("Connect error");
	pStupidBoy->addListStr(str);

	return false;
}


DWORD WINAPI manualDriveThread(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bManualDriveThreadRelease = false;

	//主线程循环
	int direction = 0;
	int speed = 0;//0 ~ 200

	int outputLayerSize = pStupidBoy->outputLayerSize;
	int *dirTable = pStupidBoy->dirTable;
	int middir = outputLayerSize/2;

	//创建显示图像线程
	HANDLE hThread = CreateThread(NULL,0,viewThread,pStupidBoy,0,0);
	
	int bufSize = pStupidBoy->jpgBufSize;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);
	pStupidBoy->addListStr(_T("ManualDrive Enabled"));
	while(pStupidBoy->bStopChildThread != true)
	{
		//互斥访问dxInput Direction
		//int len = getImage((char*)jpgBuf,pStupidBoy->remoteAddr);//获取图片
		int label = pStupidBoy->getLabel();
		speed = pStupidBoy->getSpeed();
		
		direction = getDirection(label,outputLayerSize,dirTable);
		//转向速度调整
		if(speed > 0 && label != middir)
		{
			//转向减弱
			speed -= 10;
			//转向增强
			if(pStupidBoy->bTurningSupply == true)
			{
				speed += pStupidBoy->speedSupply;
			}
		}
		//驱动车行进
		if(drive(speed,direction,pStupidBoy))
		{
			static int oldSpeed = 0;
			static int oldDirection = 0;

			//判断方向是否改变
			if(direction != oldDirection || speed != oldSpeed)
			{
				////更新记录
				oldSpeed = speed;
				oldDirection = direction;
				TCHAR str[200];
				_stprintf_s(str,_T("%5d  %5d"),speed,direction);
				pStupidBoy->addListStr(str);
			}
		}
		//延迟
		Sleep(30);
	}
	//清理
	FreeImage_CloseMemory(hMem);
	//停止
	if(drive(0,90,pStupidBoy))
	{
		TCHAR str[200];
		_stprintf_s(str,_T("%d %d"),0,90);
		pStupidBoy->addListStr(str);
	}
	CloseHandle(hThread);
	pStupidBoy->addListStr(_T("ManualDrive Exit."));
	pStupidBoy->bManualDriveThreadRelease = true;
	return 0;
}

DWORD WINAPI manualDriveForNetView(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bNetViewThreadRelease = false;

	pStupidBoy->bNetViewThreadRelease = true;
	return 0;
}


DWORD WINAPI collectDataThread(LPVOID lpParameter)
{
	//参数获取
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bCollectDataThreadRelease = false;
	//创建手动驾驶进程 
	HANDLE hThread = CreateThread(NULL,0,manualDriveThread,pStupidBoy,0,0);
	CloseHandle(hThread);
	//初始化缓存计数器
	pStupidBoy->cntBufferedData = 0;
	pStupidBoy->SetDlgItemInt(IDC_STATIC_BUFFEREDFRAMES,0);
	//互斥对象与子进程
	
	TCPClient cln;
	//char remoteAddr[100] = "127.0.0.1";
	//image param
	int width = pStupidBoy->imageWidth;
	int height = pStupidBoy->imageHeight;
	int margin = pStupidBoy->margin;

	int sleepInterval = 100;
	double framePerSecond = 2;
	int speedThreshold = pStupidBoy->speedThreshold;

	int dataBufferSize = pStupidBoy->dataBufferSize;
	int imgSize = pStupidBoy->imageHeight * pStupidBoy->imageWidth;
	MatrixXd dataSet(dataBufferSize,imgSize);
	MatrixXi labels(dataBufferSize,1);
	

	int bufSize = pStupidBoy->jpgBufSize;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);
	//allocate memory
	int *cntBuffer = new int[pStupidBoy->outputLayerSize];
	//set to 0
	memset(cntBuffer,0,sizeof(int) * pStupidBoy->outputLayerSize);

	int frame = 0;
	int timep = GetTickCount();
	srand((unsigned)time(NULL));
	int cntBufferNum = 0;
	int cntBufferChanged = 0;
	pStupidBoy->cntBufferedData = 0;
	pStupidBoy->addListStr(_T("CollectData Enabled."));
	MatrixXd recycleMat(dataSet.rows(),dataSet.cols());
	MatrixXi recycleLabels(dataSet.rows(),1);
	DList<RecycleBin> recycleList(dataSet.rows());
	int curRecyclePos = 0;
	while(pStupidBoy->bStopChildThread == false)
	{
		//do here
		//del buffer
		if(pStupidBoy->bDelBuf)
		{
			delBuf(pStupidBoy->cntBufferedData,pStupidBoy->delBufNum,
				dataSet,labels,recycleMat,recycleLabels,recycleList,curRecyclePos);
			pStupidBoy->bDelBuf = false;
		}
		//更新计数显示框
		pStupidBoy->SetDlgItemInt(IDC_STATIC_BUFFEREDFRAMES,pStupidBoy->cntBufferedData);
		int speed = pStupidBoy->getSpeed();//getDxInputSpeed(hWnd);
		//TCHAR msg[10];
		//wsprintf(msg,_T("%d"),speed);
		//pStupidBoy->addDebugStr(msg);
		if(speed < speedThreshold)
		{
			continue;
		}
		
		int len = getImage((char*)jpgBuf,pStupidBoy->remoteAddr);//获取图片
		if(len <= 0)
		{
			continue;
		}
		
		//getDataSet
		FIBITMAP *scaledImage;
		
	
		//显示图片
		CImage img;
		decodeToCImage(hMem,img);
		display(pStupidBoy->hStaticViewDC,img,pStupidBoy->displayRatio);


		//解码并缩放图片
		decodeToScaledImage(hMem,scaledImage,width,height);
		
		//获取方向label
		int dirLabel = pStupidBoy->getLabel();
		
		MatrixXd vector = decodeScaledImageToVectorF(hMem,width,height);
		//saveMat(vector,"vector.txt");
		//maintainBuffer
		maintainBuffer(vector,dirLabel,dataSet,labels,pStupidBoy->cntBufferedData,cntBuffer,pStupidBoy->outputLayerSize,
			recycleMat,recycleLabels,curRecyclePos,recycleList);
	
		//显示cntBuffer
		TCHAR szCntBuffer[200] = {0};
		for(int i = 0;i < pStupidBoy->outputLayerSize; i++)
		{
			_stprintf_s(szCntBuffer,_T("%s %d"),szCntBuffer,cntBuffer[i]);
		}
		pStupidBoy->addDebugStr(szCntBuffer);

		
		//更新帧率
		frame++;
		if(frame == 30)
		{
			frame = 0;
			int timeNow = GetTickCount();
			int frameRate = (int)(1000/(double)(timeNow - timep)*30);
			timep = timeNow;
			pStupidBoy->SetDlgItemInt(IDC_STATIC_FRAMERATE,frameRate);
		}
	}
	//释放内存
	saveDataSet("data.txt",dataSet,labels,pStupidBoy->cntBufferedData);
	FreeImage_CloseMemory(hMem);
	delete []cntBuffer;
	delete []jpgBuf;
	pStupidBoy->addListStr(_T("CollectData Exit."));
	pStupidBoy->bCollectDataThreadRelease = true;
	return 0;
}

DWORD WINAPI autoDriveThread(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bAutoDriveThreadRelease = false;
	TCPClient cln;
	//get params
	int outputLayerSize = pStupidBoy->outputLayerSize;
	int imageWidth = pStupidBoy->imageWidth;
	int imageHeight = pStupidBoy->imageHeight;
	int middir = pStupidBoy->outputLayerSize/2;
	int exampleWth = pStupidBoy->imageWidth - 2*pStupidBoy->margin;
	int inputLayerSize = exampleWth * pStupidBoy->imageHeight;
	int hiddenLayerSize = pStupidBoy->hiddenLayerSize;
	int margin = pStupidBoy->margin;

	if(imageWidth <= 0 || exampleWth <=0 || imageHeight <= 0 || 
		inputLayerSize <= 0 || hiddenLayerSize <= 0 || outputLayerSize <= 0)
	{
		AfxMessageBox(_T("Param error."));
		//cout << "Param error." << endl;
		return -1;
	}
	//创建视图驾驶线程
	HANDLE hThread = CreateThread(NULL,0,viewThread,pStupidBoy,0,0);
	CloseHandle(hThread);
	//hThread = CreateThread(NULL,0,viewSensorThread,pStupidBoy,0,0);
	//CloseHandle(hThread);
	

	int bufSize = pStupidBoy->jpgBufSize;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);

	double framePerSecond = 0;
	int timeP = GetTickCount();
	int frames = 0;
	pStupidBoy->addListStr(_T("AutoDrive Enabled."));
	//初始化稀疏自动编码器和Softmax分类器
	int netNum = pStupidBoy->netNum;
	StackedAE **pSAEs = new StackedAE*[netNum];
	Softmax softmax(1,1);
	bool bl = softmax.loadModel("Softmax_model.txt");
	if(!bl)
	{
		pStupidBoy->addListStr(_T("Softmax is not initialized."));
		return -1;
	}
	char szFileName[200];
	int i;
	//加载稀疏自动编码器权值
	for(i = 0; i < netNum; i++)
	{
		pSAEs[i] = new StackedAE(pStupidBoy->hiddenLayerSize1,pStupidBoy->hiddenLayerSize2,pStupidBoy->outputLayerSize);
		sprintf_s(szFileName,"StackedAE_Model%d.txt",i);
		bool bl1 = pSAEs[i] -> loadModel(szFileName);
		
		if(!bl1)
		{
			pStupidBoy->addListStr(_T("StackedAE not initialized"));
			return -1;
		}
	}

	TCHAR dirMapStr[200] = {0};
	for(int i =0;i < outputLayerSize;i++)
	{
		_stprintf_s(dirMapStr,_T("%s %d"),dirMapStr,pStupidBoy->dirTable[i]);
	}
	pStupidBoy->addDebugStr(dirMapStr);

	int initState = pStupidBoy->viewNetSelected;
	int goState = initState;
	bool bStateChgEn = true;
	int oldState = initState;
	int chgStateCnt = 0;
	//main loop
	while(pStupidBoy->bStopChildThread == false)
	{
		int len = getImage((char*)jpgBuf,pStupidBoy->remoteAddr);//获取图片
		if(len <= 0)
		{
			continue;
		}
		//decode to vector
		MatrixXd vector = decodeScaledImageToVectorF(hMem,
			imageWidth,imageHeight);
		
		PredStruct pStruct = {0};
		int speed = pStupidBoy->autoDriveSpeed;

		//获取传感器变道决策
		int curState = getCurPos(vector,softmax); 
		goState = sensorStrategy(pStupidBoy,curState,initState);
		//goState = testSensor(pStupidBoy->remoteAddr);
		static int oldState = -10;

		/*if(goState != oldState)
		{
			oldState = goState;
			TCHAR str[200];
			_stprintf_s(str,_T("goState %d"),goState);
			pStupidBoy->addListStr(str);
		}*/

		int p = middir;
		TCHAR sGo[20];
		_stprintf_s(sGo,_T("%d"),goState);
		pStupidBoy->addDebugStr(sGo);
		if(goState >= 0 && goState < pStupidBoy->netNum)
		{
			if(goState != initState && bStateChgEn)
			{
				oldState = goState;
				bStateChgEn = false;
				chgStateCnt = 10;
			}
			if(!bStateChgEn)
			{
				goState = oldState;
				chgStateCnt --;
				if(chgStateCnt <= 0)
				{
					bStateChgEn = true;
				}
			}
			//predict
			p = predict(*pSAEs[goState],vector);
			if(pStupidBoy->bSpeedHierarchy && p != middir)
			{
				if(speed > pStupidBoy->deltaHierarchy + 5)
				{
					speed -= pStupidBoy->deltaHierarchy;
				}
			}
		}
		else
		{
			speed = 0;
		}
	
		int relativePos = pStruct.relativePos;
		
		//转向动力补给
		if(pStupidBoy->bTurningSupply == true && p != middir)
		{
			speed += pStupidBoy->speedSupply;
		}
		int dir = getDirection(p,outputLayerSize,pStupidBoy->dirTable);
		if(drive(speed,dir,pStupidBoy))
		{
			TCHAR str[200];
			_stprintf_s(str,_T("%d %d"),speed,dir);
			pStupidBoy->addListStr(str);
		}
		pStupidBoy->updateNetView(p);
		frames ++;
		if(frames == 30)
		{
			int timeNow = GetTickCount();
			framePerSecond = 1000/(double)(timeNow - timeP) * 30;
			TCHAR strMsg[200];
			_stprintf_s(strMsg,_T("%.1lf"),framePerSecond);
			timeP = timeNow;
			frames = 0;
			pStupidBoy->SetDlgItemText(IDC_STATIC_FRAMERATE,strMsg);
		}
	}
	//释放资源
	for(i = 0; i < netNum; i++)
	{
		delete pSAEs[i];
	}
	delete pSAEs;
	FreeImage_CloseMemory(hMem);
	delete []jpgBuf;
	if(drive(0,90,pStupidBoy))
	{
		TCHAR str[200];
		_stprintf_s(str,_T("%d %d"),0,90);
		pStupidBoy->addListStr(str);
	}
	pStupidBoy->addListStr(_T("AutoDrive Exit."));
	pStupidBoy->bAutoDriveThreadRelease = true;
	return 0;
}

//高帧率自动驾驶
DWORD WINAPI autoDriveThreadF(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bAutoDriveThread_F_Release = false;
	TCPClient cln;
	//get params
	int outputLayerSize = pStupidBoy->outputLayerSize;
	int imageWidth = pStupidBoy->imageWidth;
	int imageHeight = pStupidBoy->imageHeight;
	int middir = pStupidBoy->outputLayerSize/2;
	int exampleWth = pStupidBoy->imageWidth - 2*pStupidBoy->margin;
	int inputLayerSize = exampleWth * pStupidBoy->imageHeight;
	int hiddenLayerSize = pStupidBoy->hiddenLayerSize;
	int margin = pStupidBoy->margin;

	if(imageWidth <= 0 || exampleWth <=0 || imageHeight <= 0 || 
		inputLayerSize <= 0 || hiddenLayerSize <= 0 || outputLayerSize <= 0)
	{
		AfxMessageBox(_T("Param error."));
		//cout << "Param error." << endl;
		return -1;
	}
	//创建视图驾驶线程
	HANDLE hThread = CreateThread(NULL,0,viewThread,pStupidBoy,0,0);
	CloseHandle(hThread);
	

	int bufSize = pStupidBoy->jpgBufSize;
	unsigned char *jpgBuf = new unsigned char[bufSize];
	FIMEMORY *hMem = FreeImage_OpenMemory((BYTE*)jpgBuf, bufSize);

	double framePerSecond = 0;
	int timeP = GetTickCount();
	int frames = 0;
	pStupidBoy->addListStr(_T("AutoDrive-F Enabled."));
	//初始化稀疏自动编码器和Softmax分类器
	int netNum = pStupidBoy->netNum;
	StackedAE **pSAEs = new StackedAE*[netNum];
	/*Softmax softmax(1,1);
	bool bl = softmax.loadModel("Softmax_model.txt");
	if(!bl)
	{
		pStupidBoy->addListStr(_T("Softmax is not initialized."));
		return -1;
	}*/
	char szFileName[200];
	int i;
	//加载稀疏自动编码器权值
	for(i = 0; i < netNum; i++)
	{
		pSAEs[i] = new StackedAE(pStupidBoy->hiddenLayerSize1,pStupidBoy->hiddenLayerSize2,pStupidBoy->outputLayerSize);
		sprintf_s(szFileName,"StackedAE_Model%d.txt",i);
		bool bl1 = pSAEs[i] -> loadModel(szFileName);
		
		if(!bl1)
		{
			pStupidBoy->addListStr(_T("StackedAE not initialized"));
			return -1;
		}
	}

	TCHAR dirMapStr[200] = {0};
	for(int i =0;i < outputLayerSize;i++)
	{
		_stprintf_s(dirMapStr,_T("%s %d"),dirMapStr,pStupidBoy->dirTable[i]);
	}
	pStupidBoy->addDebugStr(dirMapStr);

	int initState = pStupidBoy->viewNetSelected;
	int goState = initState;
	//main loop
	while(pStupidBoy->bStopChildThread == false)
	{
		int len = getImage((char*)jpgBuf,pStupidBoy->remoteAddr);//获取图片
		if(len <= 0)
		{
			continue;
		}
		//decode to vector
		MatrixXd vector = decodeScaledImageToVectorF(hMem,
			imageWidth,imageHeight);
		
		PredStruct pStruct = {0};
		int speed = pStupidBoy->autoDriveSpeed;

		//获取传感器变道决策
		//int curState = getCurPos(vector,softmax);
		int curState = initState;
		//goState = sensorStrategy(pStupidBoy,curState,initState);
		//goState = testSensor(pStupidBoy->remoteAddr);
		static int oldState = -10;

		int p = middir;
		TCHAR sGo[20];
		_stprintf_s(sGo,_T("%d"),goState);
		pStupidBoy->addDebugStr(sGo);
		if(goState >= 0 && goState < pStupidBoy->netNum)
		{
			//predict
			p = predict(*pSAEs[goState],vector);
			if(pStupidBoy->bSpeedHierarchy && p != middir)
			{
				if(speed > pStupidBoy->deltaHierarchy + 5)
				{
					speed -= pStupidBoy->deltaHierarchy;
				}
			}
		}
		else
		{
			speed = 0;
		}
	
		int relativePos = pStruct.relativePos;
		
		//转向动力补给
		if(pStupidBoy->bTurningSupply == true && p != middir)
		{
			speed += pStupidBoy->speedSupply;
		}
		int dir = getDirection(p,outputLayerSize,pStupidBoy->dirTable);
		if(drive(speed,dir,pStupidBoy))
		{
			TCHAR str[200];
			_stprintf_s(str,_T("%d %d"),speed,dir);
			pStupidBoy->addListStr(str);
		}
		pStupidBoy->updateNetView(p);
		frames ++;
		if(frames == 30)
		{
			int timeNow = GetTickCount();
			framePerSecond = 1000/(double)(timeNow - timeP) * 30;
			TCHAR strMsg[200];
			_stprintf_s(strMsg,_T("%.1lf"),framePerSecond);
			timeP = timeNow;
			frames = 0;
			pStupidBoy->SetDlgItemText(IDC_STATIC_FRAMERATE,strMsg);
		}
	}
	//释放资源
	for(i = 0; i < netNum; i++)
	{
		delete pSAEs[i];
	}
	delete pSAEs;
	FreeImage_CloseMemory(hMem);
	delete []jpgBuf;
	if(drive(0,90,pStupidBoy))
	{
		TCHAR str[200];
		_stprintf_s(str,_T("%d %d"),0,90);
		pStupidBoy->addListStr(str);
	}
	pStupidBoy->addListStr(_T("AutoDrive-F Exit."));
	pStupidBoy->bAutoDriveThread_F_Release = true;
	return 0;
}

DWORD WINAPI trainThread(LPVOID lpParameter)
{
	//get main dialog class 
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;

	int hiddenSize = pStupidBoy->hiddenLayerSize;//number of hidden layers
	double saeLambda = 0.01;//l2 regularization of sparse autoencoder
	double saeAlpha = 0.1;//learning rate of sparse autoencoder
	double saeBeta = 1e-3;//sparsity term relative weight
	double saeSparsityParam = 0.01;//sparsity param
	int saeMaxIter = 200;//iterations of training sparse autoencoder
	int saeMiniBatchSize = 100;//mini batchsize of training
	double softmaxLambda = 0.01;//l2 regualarization of softmax model
	double softmaxAlpha = 0.01;//learning rate of softmax model
	int softmaxMaxIter = 200;//iterations of training softmax model
	//load parameters
	char *fileBuf  = new char[4096];
	bool ret = loadFileToBuf("ParamConfig.ini",fileBuf,4096);
	if(ret)
	{
		getConfigDoubleValue(fileBuf,"saeLambda:",saeLambda);
		getConfigDoubleValue(fileBuf,"saeAlpha:",saeAlpha);
		getConfigDoubleValue(fileBuf,"saeBeta:",saeBeta);
		getConfigDoubleValue(fileBuf,"saeSparsityParam:",saeSparsityParam);
		getConfigIntValue(fileBuf,"saeMaxIter:",saeMaxIter);
		getConfigIntValue(fileBuf,"saeMiniBatchSize:",saeMiniBatchSize);
		getConfigDoubleValue(fileBuf,"softmaxLambda:",softmaxLambda);
		getConfigDoubleValue(fileBuf,"softmaxAlpha:",softmaxAlpha);
		getConfigIntValue(fileBuf,"smIter:",softmaxMaxIter);
	}
	delete []fileBuf;

	MatrixXd dataSet(1,1);
	MatrixXi labels(1,1);
	//load training dataset
	pStupidBoy->addListStr(_T("Loading trainning set..."));
	if(!loadDataSet("data.txt",dataSet,labels))
	{
		return -1;
	}
	//MatrixXd testImage = dataSet.topRows(1);
	//buildImage(testImage,48,"testImage.jpg");
	//get no margin images
	MatrixXd dataSetNoMargin = getNoMarginDataSet(dataSet,48,pStupidBoy->margin);
	MatrixXd testImageNoMargin = dataSetNoMargin.topRows(9);
	//buildImage(testImageNoMargin,48 - 2 * pStupidBoy->margin,"testImageNoMargin.jpg");
	MatrixXd saeTrainData = dataSetNoMargin.transpose();
	pStupidBoy->addListStr(_T("Trainning saeModel..."));
	addLog(saeSparsityParam);
	SAE sae(saeTrainData.rows(),hiddenSize);
	//train a sparse autoencoder
	sae.train(saeTrainData,saeLambda,saeAlpha,saeBeta,
		saeSparsityParam,saeMaxIter,saeMiniBatchSize);
	MatrixXd theta = sae.getTheta();
	int imageWidth = pStupidBoy->imageWidth;
	int filterWidth = imageWidth - 2 * (pStupidBoy -> margin);
	buildImage(theta,filterWidth,"theta.jpg");
	sae.saveModel("saeModel.txt");
	
	//do conv and pooling
	pStupidBoy->addListStr(_T("conv pooling..."));
	//TCHAR szDebug[100];
	MatrixXd convPooledFeature = getConvPooledDataSet(dataSet,theta,imageWidth,
		filterWidth,pStupidBoy->convStep);
	//_stprintf_s(szDebug,_T("%d %d"),convPooledFeature.rows(),convPooledFeature.cols());
	//pStupidBoy->addListStr(szDebug);
	Softmax softmax(convPooledFeature.cols(),pStupidBoy->outputLayerSize);

	
	//_stprintf_s(szDebug,_T("%d %d"),convPooledFeature.cols(),pStupidBoy->outputLayerSize);
	//pStupidBoy->addListStr(szDebug);

	pStupidBoy->addListStr(_T("Trainning softmaxModel..."));
	MatrixXd softmaxData = convPooledFeature.transpose();


	
	pStupidBoy->addListStr(_T("softmax train..."));
	softmax.train(softmaxData,labels,softmaxLambda,softmaxAlpha,softmaxMaxIter,convPooledFeature.rows());
	//MatrixXd softmaxTheta = softmax.getTheta();
	softmax.saveModel("softmaxModel.txt");
	pStupidBoy->addListStr(_T("Finished training."));
	return 0;
}

DWORD WINAPI viewSensorThread(LPVOID lpParameter)
{
	//get main dialog class 
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bViewSensorThreadRelease = false;
	pStupidBoy->addListStr(_T("View Sensor Enabled."));
	int initState = pStupidBoy->initNetSelected;
	int goState = initState;
	while(pStupidBoy->bStopChildThread == false)
	{
		int data[361] = {0};
		if(getSensorData(data,pStupidBoy->remoteAddr))
		{
			TCHAR str[100] = {0};
			_stprintf_s(str,_T("%d %d %d %d %d %d %d %d %d"),
				data[0],data[1],data[2],data[3],data[4],
				data[5],data[6],data[7],data[8]);
			pStupidBoy->addDebugStr(str);
			/*goState = sensorStrategy(pStupidBoy->remoteAddr,goState,initState);
			_stprintf_s(str,_T("goState: %d"),goState);
			pStupidBoy->addDebugStr(str);*/
		}
		Sleep(30);
	}
	pStupidBoy->addListStr(_T("View Sensor Exit."));
	pStupidBoy->bViewSensorThreadRelease = true;
	return 0;
}

DWORD WINAPI updateSensorThread(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bUpdateSensorThreadRelease = false;
	while(pStupidBoy->bStopDxInput == false)
	{
		int dataArr[361] = {0};
		//get data
		getSensorData(dataArr,pStupidBoy->remoteAddr);
		bool frontState = getFrontState(dataArr);
		bool frontLeftState = getFrontLeftState(dataArr);
		bool frontRightState = getFrontRightState(dataArr);
		bool leftState = getLeftState(dataArr);
		bool rightState = getRightState(dataArr);
		bool frontObstacle = getFrontObstacle(dataArr);
		bool frontLeftObstacle = getFrontLeftObstacle(dataArr);
		bool frontRightObstacle = getFrontRightObstacle(dataArr);
		bool leftObstacle = getLeftObstacle(dataArr);
		bool rightObstacle = getRightObstacle(dataArr);
		//update
		EnterCriticalSection(&(pStupidBoy->sensorCS));
		pStupidBoy->frontState = frontState;
		pStupidBoy->frontRightState = frontRightState;
		pStupidBoy->frontLeftState = frontLeftState;
		pStupidBoy->leftState = leftState;
		pStupidBoy->rightState = rightState;
		pStupidBoy->frontObstacle = frontObstacle;
		pStupidBoy->frontLeftObstacle = frontLeftObstacle;
		pStupidBoy->frontRightObstacle = frontRightObstacle;
		pStupidBoy->leftObstacle = leftObstacle;
		pStupidBoy->rightObstacle = rightObstacle;
		LeaveCriticalSection(&(pStupidBoy->sensorCS));
	}
	pStupidBoy->bUpdateSensorThreadRelease = true;
	return 0;
}


DWORD WINAPI waitThread(LPVOID lpParameter)
{
	CstupidBoyDlg *pStupidBoy = (CstupidBoyDlg*)lpParameter;
	pStupidBoy->bStopChildThread = true;

	while(!(pStupidBoy->bViewThreadRelease && pStupidBoy->bViewSensorThreadRelease
		&& pStupidBoy->bCollectDataThreadRelease && pStupidBoy->bManualDriveThreadRelease
		&& pStupidBoy->bAutoDriveThreadRelease && pStupidBoy->bAutoDriveThread_F_Release 
		&& pStupidBoy->bNetViewThreadRelease))
	{
		SwitchToThread();
	}
	return 0;
}