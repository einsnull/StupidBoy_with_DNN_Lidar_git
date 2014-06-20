#pragma once
#include "stdafx.h"
#include "stupidBoyDlg.h"
#include "tcpClient.h"
#include "myString.h"
#include "FreeImage.h"


void buildImage(MatrixXd &theta,int imgWidth,char* szFileName,bool showFlag = false,double ratio = 1);

//�߶ȱ任
bool Scaling(CImage& srcImg,CImage& dstImg,double ratio);

bool Scaling(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight);

bool scaleImage(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight);

//��ʾͼƬ��DC��ratioΪ��ʾ�ٷֱ�
void display(HDC hdc,CImage &image,double ratio);

//Զ�̻�ȡͼ���С
int getRemoteImageSize(char *remoteAddr,int &width,int &height,int margin);

//functions for region grow

//��ȡ�Ҷ�ֵ
int inline getGray(CImage &img,int x,int y);

//��ȡ��ռ����
double getRatio(CImage &img);

//get gray value of weighted neighbor
double getNeighborGray(CImage &img,int x,int y);

//region grow
bool fastRegionGrow(CImage& srcImg,CImage &dstImg, int nSeedX, int nSeedY,double tolerance = 1);

bool decodeToCImage(FIMEMORY *hMem,CImage &image);//decodeJpeg

bool decodeToGrayCImage(FIMEMORY *hMem,CImage &image);

//�������ݣ����Ҹ�����margin������
bool copyImgDataToCImage(CImage &srcImage,CImage &dstImage,int margin);

//scale image
void scaleImage(FIBITMAP *src,FIBITMAP *dst,int dstWidth,int dstHeight);

//���벢����ͼƬ
void decodeToScaledImage(FIMEMORY *hMem,FIBITMAP* &dstImage,int dstWidth,int dstHeight);