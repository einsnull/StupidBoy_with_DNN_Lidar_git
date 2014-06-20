#pragma once
#include "stdafx.h"
#include "stupidBoyDlg.h"
#include "tcpClient.h"
#include "myString.h"
#include "FreeImage.h"


void buildImage(MatrixXd &theta,int imgWidth,char* szFileName,bool showFlag = false,double ratio = 1);

//尺度变换
bool Scaling(CImage& srcImg,CImage& dstImg,double ratio);

bool Scaling(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight);

bool scaleImage(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight);

//显示图片到DC，ratio为显示百分比
void display(HDC hdc,CImage &image,double ratio);

//远程获取图像大小
int getRemoteImageSize(char *remoteAddr,int &width,int &height,int margin);

//functions for region grow

//获取灰度值
int inline getGray(CImage &img,int x,int y);

//获取所占比例
double getRatio(CImage &img);

//get gray value of weighted neighbor
double getNeighborGray(CImage &img,int x,int y);

//region grow
bool fastRegionGrow(CImage& srcImg,CImage &dstImg, int nSeedX, int nSeedY,double tolerance = 1);

bool decodeToCImage(FIMEMORY *hMem,CImage &image);//decodeJpeg

bool decodeToGrayCImage(FIMEMORY *hMem,CImage &image);

//拷贝数据，左右各留出margin个像素
bool copyImgDataToCImage(CImage &srcImage,CImage &dstImage,int margin);

//scale image
void scaleImage(FIBITMAP *src,FIBITMAP *dst,int dstWidth,int dstHeight);

//解码并缩放图片
void decodeToScaledImage(FIMEMORY *hMem,FIBITMAP* &dstImage,int dstWidth,int dstHeight);