#pragma once
#include "TcpClient.h"
#include "AddLog.h"
#include "stupidBoyDlg.h"
//远程获取图片
int getImage(char *buf,char *addr);

bool getSensorData(int *dataArr,char *addr);

//获取图片大小
int getRemoteImageSize(char *remoteAddr,int &width,int &height,int margin);

//获取状态
bool getFrontState(int *data);
//获取距离传感器数据
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

//传感器策略
int sensorStrategy(CstupidBoyDlg *pStupidBoy,int curState,int initState);