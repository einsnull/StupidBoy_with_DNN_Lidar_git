#pragma once
#include "TcpClient.h"
#include "AddLog.h"
#include "stupidBoyDlg.h"
//Զ�̻�ȡͼƬ
int getImage(char *buf,char *addr);

bool getSensorData(int *dataArr,char *addr);

//��ȡͼƬ��С
int getRemoteImageSize(char *remoteAddr,int &width,int &height,int margin);

//��ȡ״̬
bool getFrontState(int *data);
//��ȡ���봫��������
bool getFrontObstacle(int *data);

bool getLeftObstacle(int *data);

bool getRightObstacle(int *data);

bool getFrontRightObstacle(int *data);

bool getFrontLeftObstacle(int *data);

bool getLeftState(int *data);

bool getRightState(int *data);

bool getFrontRightState(int *data);

bool getFrontLeftState(int *data);

int testSensor(char *addr);

//����������
int sensorStrategy(CstupidBoyDlg *pStupidBoy,int curState,int initState);