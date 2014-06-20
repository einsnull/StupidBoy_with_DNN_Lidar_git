#include "tcpClient.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>


#if _DEBUG
#pragma comment(lib,"opencv_core246d.lib")
#pragma comment(lib,"opencv_highgui246d.lib")
#pragma comment(lib,"opencv_imgproc246d.lib")
#else
#pragma comment(lib,"opencv_core246.lib")
#pragma comment(lib,"opencv_highgui246.lib")
#pragma comment(lib,"opencv_imgproc246.lib")
#endif

int main()
{
	const double PI = 3.14159265;
	int maxWidth = 1200;
	int maxHeight = 700;
	IplImage *image = cvCreateImage(cvSize(maxWidth,maxHeight),IPL_DEPTH_8U,3);
	int centerX = image->width / 2;
	int centerY = image->height / 2;
	CvPoint center;
	center.x = centerX;
	center.y = centerY;
	CvPoint point;
	int height = image->height;
	int width = image->width;
	int step = image->widthStep/sizeof(uchar);
	int channels = image->nChannels;
	uchar *data = (uchar *)image->imageData;
	cvNamedWindow("Lidar Data:");
	TCPClient client;
    // fetech result and print it out...
    while (1) {

		//set background
		for(int h = 0; h < height; h++)
		{
			for(int w = 0; w < width; w++)
			{
				for(int k = 0;k < channels; k++)
				{
					data[h * step + w * channels + k] = 0;
				}
			}
		}
		
		unsigned char buf[361] = {0};
		client.connectHost("192.168.1.115",2015);
		int count = client.recvData((char*)buf,360);
		client.disconnect();

		for (int pos = 0; pos < (int)count ; ++pos) {
			double dist = buf[pos];
			double rad = pos * PI / 180;
			double endptX = sin(rad)*(dist) + centerX;
			double endptY = centerY - cos(rad)*(dist);
			if(endptX > maxWidth)
			{
				endptX = maxWidth;
			}
			if(endptX < 0)
			{
				endptX = 0;
			}
			if(endptY > maxHeight)
			{
				endptY = maxHeight;
			}
			if(endptY < 0)
			{
				endptY = 0;
			}
			point.x = (int)endptX;
			point.y = (int)endptY;
			//129,10,16
			cvCircle(image,point,3,cvScalar(16,10,129),-1);
			cvLine(image,center,point,cvScalar(16,10,129),1,1);
		}

		cvCircle(image,center,10,cvScalar(0,0,255),-1);
		cvShowImage("Lidar Data:",image);
		char c = cvWaitKey(1);
		if(c == VK_ESCAPE)
		{
			break;
		}
    }
	cvDestroyWindow("Lidar Data:");
	return 0;
}