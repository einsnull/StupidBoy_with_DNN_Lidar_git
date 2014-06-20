#pragma once
#include "stdafx.h"
#include "imageFunctions.h"
#include <opencv2/opencv.hpp>
#include <queue>
using namespace std;

#pragma comment(lib,"FreeImage.lib")
#if _DEBUG
#pragma comment(lib,"opencv_core246d.lib")
#pragma comment(lib,"opencv_highgui246d.lib")
#pragma comment(lib,"opencv_imgproc246d.lib")
#else
#pragma comment(lib,"opencv_core246.lib")
#pragma comment(lib,"opencv_highgui246.lib")
#pragma comment(lib,"opencv_imgproc246.lib")
#endif

typedef struct{
	int x;
	int y;
} Elem;

//尺度变换
bool Scaling(CImage& srcImg,CImage& dstImg,double ratio)
{
	int nHeight = srcImg.GetHeight();
	int nWidth = srcImg.GetWidth();
	
	if(dstImg.CreateEx((int)(nWidth*ratio),(int)(nHeight*ratio), 24, BI_RGB))
	{
		HDC ImgDC = dstImg.GetDC();
		int nPreMode = ::SetStretchBltMode(ImgDC,  HALFTONE);
		srcImg.Draw(ImgDC, 0, 0, (int)(nWidth*ratio),(int)(nHeight*ratio), 0, 0, nWidth, nHeight);
		::SetBrushOrgEx(ImgDC, 0, 0, NULL); 
		::SetStretchBltMode(ImgDC, nPreMode);
		dstImg.ReleaseDC();
	}
	else
	{
		return false;
	}
	return true;
}

bool Scaling(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight)
{
	int nHeight = srcImg.GetHeight();
	int nWidth = srcImg.GetWidth();
	
	if(dstImg.CreateEx(dstWidth,dstHeight, 24, BI_RGB))
	{
		HDC ImgDC = dstImg.GetDC();
		int nPreMode = ::SetStretchBltMode(ImgDC,  HALFTONE);
		srcImg.Draw(ImgDC, 0, 0, dstWidth,dstHeight, 0, 0, nWidth, nHeight);
		::SetBrushOrgEx(ImgDC, 0, 0, NULL); 
		::SetStretchBltMode(ImgDC, nPreMode);
		dstImg.ReleaseDC();
	}
	else
	{
		return false;
	}
	return true;
}

bool scaleImage(CImage& srcImg,CImage& dstImg,int dstWidth,int dstHeight)
{
	int nHeight = srcImg.GetHeight();
	int nWidth = srcImg.GetWidth();
	
	if(dstImg.CreateEx(dstWidth,dstHeight, 24, BI_RGB))
	{
		HDC ImgDC = dstImg.GetDC();
		int nPreMode = ::SetStretchBltMode(ImgDC,  HALFTONE);
		srcImg.Draw(ImgDC, 0, 0, dstWidth,dstHeight, 0, 0, nWidth, nHeight);
		::SetBrushOrgEx(ImgDC, 0, 0, NULL); 
		::SetStretchBltMode(ImgDC, nPreMode);
		dstImg.ReleaseDC();
	}
	else
	{
		return false;
	}
	return true;
}

//scale image
void scaleImage(FIBITMAP *src,FIBITMAP *dst,int dstWidth,int dstHeight)
{
	dst = FreeImage_Rescale(src,dstWidth,dstHeight,FILTER_BOX);
}

//显示图片到DC，ratio为显示百分比
void display(HDC hdc,CImage &image,double ratio)
{
	CImage dstImg;
	Scaling(image,dstImg,ratio);
	dstImg.BitBlt(hdc,0,0,dstImg.GetWidth(),dstImg.GetHeight(),0,0,SRCCOPY);
}

//获取像素灰度
int getGray(CImage &img,int x,int y)
{
	int r,g,b;
	int height = img.GetHeight();
	int width = img.GetWidth();
	if(x < 0 || x >= width || y < 0 || y >= height )
	{
		return -1;
	}
	BYTE *p = (BYTE*)img.GetPixelAddress(x,y);
	b = *p++;
	g = *p++;
	r = *p;
	//灰度值计算公式
	return (int)(0.30*r + 0.59*g + 0.11*b);
}

//decodeJpeg
bool decodeToCImage(FIMEMORY *hMem,CImage &image)
{
	//设置内存流指针到初始位置
	FreeImage_SeekMemory(hMem,0,SEEK_SET);
	FIBITMAP *check = FreeImage_LoadFromMemory(FIF_JPEG, hMem, 0);
	unsigned int width = FreeImage_GetWidth(check);
	unsigned int height = FreeImage_GetHeight(check);
	unsigned int pitch = FreeImage_GetPitch(check);
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(check);
	if(!image.CreateEx(width,height,24,BI_RGB))
	{
		FreeImage_Unload(check);
		return false;
	}
	int x,y;
	if((image_type == FIT_BITMAP) && (FreeImage_GetBPP(check) == 24)) 
	{
		//像素操作
		BYTE *bits = (BYTE*)FreeImage_GetBits(check);
		for(y = 0; y < (int)height; y++)
		{
			BYTE *pixel = (BYTE*)bits;
			for(x = 0; x < (int)width; x++) 
			{
				BYTE *p = (BYTE*)image.GetPixelAddress(x, height -1 - y);
				*p++ = pixel[FI_RGBA_BLUE];
				*p++ = pixel[FI_RGBA_GREEN];
				*p++ = pixel[FI_RGBA_RED];
				pixel += 3;
			}
			// next line
			bits += pitch;
		}
	}
	else
	{
		FreeImage_Unload(check);
		return false;
	}
	FreeImage_Unload(check);
	return true;
}

//decodeJpeg to gray image
bool decodeToGrayCImage(FIMEMORY *hMem,CImage &image)
{
	FreeImage_SeekMemory(hMem,0,SEEK_SET);
	FIBITMAP *check = FreeImage_LoadFromMemory(FIF_JPEG, hMem, 0);
	unsigned int width = FreeImage_GetWidth(check);
	unsigned int height = FreeImage_GetHeight(check);
	unsigned int pitch = FreeImage_GetPitch(check);
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(check);
	if(!image.CreateEx(width,height,24,BI_RGB))
	{
		FreeImage_Unload(check);
		return false;
	}
	int x,y;
	if((image_type == FIT_BITMAP) && (FreeImage_GetBPP(check) == 24)) 
	{
		//像素操作
		BYTE *bits = (BYTE*)FreeImage_GetBits(check);
		for(y = 0; y < (int)height; y++) 
		{
			BYTE *pixel = (BYTE*)bits;
			for(x = 0; x < (int)width; x++) 
			{
				BYTE *p = (BYTE*)image.GetPixelAddress(x, height -1 - y);
				int grayVal = (int)(0.30 * pixel[FI_RGBA_RED] + 0.59 * pixel[FI_RGBA_GREEN] + 0.11*pixel[FI_RGBA_BLUE]);
				*p++ = (BYTE)grayVal;
				*p++ = (BYTE)grayVal;
				*p++ = (BYTE)grayVal;
				pixel += 3;
			}
			// next line
			bits += pitch;
		}
	}
	else
	{
		FreeImage_Unload(check);
		return false;
	}
	FreeImage_Unload(check);
	return true;
}

bool copyImgDataToCImage(CImage &srcImage,CImage &dstImage,int margin)
{
	margin  = abs(margin);
	//读取图像宽高
	int width = srcImage.GetWidth();
	int height = srcImage.GetHeight();
	if(margin*2 > width)
	{
		return false;
	}
	//new width
	int imgWidth = width-2*margin;

	if(!dstImage.CreateEx(imgWidth,height, 24, BI_RGB))//创建CImage 对象
	{
		return false;
	}
	//设置像素
	for(int i = height - 1;i >= 0 ; i--)
	{
		for(int j = 0; j < imgWidth;j++)
		{
			BYTE *pDst = (BYTE*)dstImage.GetPixelAddress(j, i);
			BYTE *pSrc = (BYTE*)srcImage.GetPixelAddress(j + margin, i);
			//读取并设置 rgb值
			*pDst++ = *pSrc++; // b分量值
			*pDst++ = *pSrc++; //g 分量
			*pDst = *pSrc; // r分量
		}
		//new offset
	}
	return true;
}


//解码并缩放图片
void decodeToScaledImage(FIMEMORY *hMem,FIBITMAP* &dstImage,int dstWidth,int dstHeight)
{
	//设置内存流指针到起始位置
	FreeImage_SeekMemory(hMem,0,SEEK_SET);
	//从内存流载入数据
	FIBITMAP *check = FreeImage_LoadFromMemory(FIF_JPEG, hMem, 0);
	dstImage = FreeImage_Rescale(check,dstWidth,dstHeight,FILTER_BOX);
	FreeImage_Unload(check);
}

void buildImage(MatrixXd &theta,int imgWidth,char* szFileName,bool showFlag,double ratio)
{
	int margin = 1;
	int rows = theta.rows();
	int cols = theta.cols();
	if(rows <= 0 || cols <= 0)
	{
		return ;
	}
	/*cout << rows << endl;
	cout << cols << endl;*/
	double pr = sqrt((double)rows);
	int perRow = (int)pr + (pr - (int)pr > 0);
	double tc = (double)rows / (double)perRow;
	int tCols = (int)tc + (tc - (int)tc > 0);
	/*cout << "perRow: " << perRow << endl;
	cout << "tCols: " << tCols << endl;*/
	MatrixXd max = theta.rowwise().maxCoeff();
	MatrixXd min = theta.rowwise().minCoeff();

	int imgHeight = cols/imgWidth;
	IplImage* iplImage = cvCreateImage(
		cvSize(imgWidth * perRow + margin * (perRow+1),imgHeight * tCols + margin * (tCols + 1)),
		IPL_DEPTH_8U,1);
	
	int step = iplImage->widthStep;
	uchar *data = (uchar *)iplImage->imageData;
	int h = iplImage->height;
	int w = iplImage->width;
	for(int x = 0; x < w; x++)
	{
		for(int y = 0; y < h; y++)
		{
			data[y * step + x] = 0;
		}
	}

	for(int i = 0;i < rows;i++)
	{
		
		int n = 0;
		int hIdx = i / perRow;
		int wIdx = i % perRow;

		for(int j = 0;j < imgHeight;j++)
		{
			for(int k = 0;k < imgWidth; k++)
			{
				double per = (theta(i,n) - min(i,0) ) / (max(i,0) - min(i,0));
				//data[j * step + k] = 255;
				int val = (hIdx * imgHeight+j + margin * (hIdx+1)) * step
					+ (wIdx * imgWidth + k + margin * (wIdx+1));
				if(val > step * (imgHeight * tCols + margin * (tCols + 1)))
				{
					cout << "error" << endl;
				}
				data[val] = (uchar)(int)(per * 230.0);
				n ++;
			}
		}
		
	}
	
	cvSaveImage(szFileName,iplImage);

	if(showFlag)
	{
		cvNamedWindow("Image",CV_WINDOW_AUTOSIZE);
		IplImage* iplImageShow = cvCreateImage(
			cvSize((int)(iplImage->width * ratio),(int)(iplImage->height * ratio)),IPL_DEPTH_8U,1);
		cvResize(iplImage,iplImageShow,CV_INTER_CUBIC);
		cvShowImage("Image",iplImageShow);
		cvWaitKey(100000);
		cvDestroyWindow("Image");
		cvReleaseImage(&iplImageShow);
	}
	cvReleaseImage(&iplImage);
}