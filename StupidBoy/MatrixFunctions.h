#pragma once
#include <cassert>
#include "imageFunctions.h"
#include <fstream>
#include <Eigen/Dense>
#include "SAE.h"
#include "Softmax.h"
#include "StackedAE.h"
#include "AddLog.h"
#include "DList.h"
using namespace std;
using namespace Eigen;

typedef struct
{
	int bufferId;
	int binId;
} RecycleBin;

typedef struct{
	double val;
	int id;
} PoolStruct;

typedef struct{
	int val;
	int relativePos;
} PredStruct;
//matrix functions


//数据回收
void recycleBuf(MatrixXd &recycleMat,MatrixXi &recycleLabels,
				MatrixXd &dataset,MatrixXi &labels,int recycleId,
				int &curRecyclePos,DList<RecycleBin> &list)
{
	int id = curRecyclePos % list.getMaxCnt();
	curRecyclePos %= list.getMaxCnt();
	//数据回收位置索引
	RecycleBin bin;
	bin.bufferId = recycleId;
	bin.binId = id;
	list.addTail(bin);
	//将数据放入回收站
	recycleMat.middleRows(id,1) = dataset.middleRows(recycleId,1);
	recycleLabels(id,0) = labels(recycleId,0);
	curRecyclePos++;
}

//数据恢复
void restoreBuf(MatrixXd &dataset,MatrixXi &labels,
				MatrixXd &recycleMat,MatrixXi &recycleLabels,
				DList<RecycleBin> &list,int &restoreNum,
				int &curRecyclePos)
{
	//获取回收站中的数据总数
	int cnt = list.getCnt();
	int maxRecycled = restoreNum;
	if(restoreNum > cnt)
	{
		maxRecycled = cnt;
	}
	//从回收站中恢复数据
	for(int i = 0;i < maxRecycled;i++)
	{
		RecycleBin bin;
		if(list.tailValue(bin))
		{
			dataset.middleRows(bin.bufferId,1) = 
				recycleMat.middleRows(bin.binId,1);
			labels(bin.bufferId,0) = recycleLabels(bin.binId,0);
			list.delTail();
		}
	}
	curRecyclePos = list.getCnt();
}


void delBuf(int &bufCnt,int delNum,
			MatrixXd &dataset,MatrixXi &labels,
			MatrixXd &recycleMat,MatrixXi &recycleLabels,
			DList<RecycleBin> &list,int &curRecyclePos)
{
	if(list.getCnt() <= delNum)
	{
		bufCnt = bufCnt - delNum + list.getCnt();
		if(bufCnt < 0)
		{
			bufCnt = 0;
		}
	}
	restoreBuf(dataset,labels,recycleMat,recycleLabels,
		list,delNum,curRecyclePos);
}

inline int getMaxCntBuffer(int *cntBuffer,int labelsNum)
{
	int id;
	int max = 0;
	for(int i = 0; i < labelsNum; i++)
	{
		if(cntBuffer[i] > max)
		{
			max = cntBuffer[i];
			id = i;
		}
	}
	return id;
}

void replaceBuffer(MatrixXd &dataset,MatrixXi &labels,
				   MatrixXd &vector,int label,int maxLabelID)
{
	for(int i = 0; i < labels.rows(); i++)
	{
		if(labels(i,0) == maxLabelID)
		{
			//do replace
			dataset.middleRows(i,1) = vector;
			labels(i,0) = label;
			return;
		}
	}
}

void maintainBuffer(MatrixXd &vector,int label,MatrixXd &dataset,
					MatrixXi &labels,int &bufferCnt,int *cntBuffer,
					int labelsNum,MatrixXd &recycleMat,
					MatrixXi &recycleLabels,int &curRecyclePos,
					DList<RecycleBin> &list)
{
	int bufferSize = dataset.rows();
	int curBufferPos = bufferCnt;

	if(curBufferPos < bufferSize)
	{
		dataset.middleRows(curBufferPos,1) = vector;
		labels(curBufferPos,0) = label;
		cntBuffer[label] ++;
		bufferCnt ++;
	}
	else
	{
		int id = getMaxCntBuffer(cntBuffer,labelsNum);
		//recycle first
		recycleBuf(recycleMat,recycleLabels,dataset,labels,id,
			curRecyclePos,list);
		//replace buffer
		replaceBuffer(dataset,labels,vector,label,id);
		cntBuffer[label] ++;
		cntBuffer[id] --;
	}
}


MatrixXd decodeScaledImageToVectorF(FIMEMORY *hmem,int dstWidth,int dstHeight,int margin = 0)//decodeJpeg
{
	if(dstWidth - 2*margin <= 0)
	{
		assert(dstWidth - 2*margin > 0);
		return MatrixXd::Zero(1,1);
	}
	//设置内存流指针到起始位置
	FreeImage_SeekMemory(hmem,0,SEEK_SET);
	//从内存流载入数据
	FIBITMAP *check = FreeImage_LoadFromMemory(FIF_JPEG, hmem, 0);
	//获取宽度，高度以及pitch
	FIBITMAP *dst = FreeImage_Rescale(check,dstWidth,dstHeight,FILTER_BOX);
	unsigned int pitch = FreeImage_GetPitch(dst);
	//获取编码类型
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dst);
	int x,y;
	int idx = 0;
	MatrixXd vector = MatrixXd::Zero(1,dstHeight * (dstWidth - 2 * margin));
	if((image_type == FIT_BITMAP) && (FreeImage_GetBPP(dst) == 24)) 
	{
		//像素操作
		BYTE *bits = (BYTE*)FreeImage_GetBits(dst);
	
		for(y = (int)dstHeight - 1; y >= 0; y--)
		{
			for(x = margin; x < (int)dstWidth - margin; x++)  
			{
				BYTE *pixel = bits + y*pitch + x*3; 
				int grayVal = (int)(0.30 * pixel[FI_RGBA_RED] + 0.59 * pixel[FI_RGBA_GREEN] + 0.11*pixel[FI_RGBA_BLUE]);
				double dVal = (double)grayVal / (double)255;
				vector(0,idx++) = dVal;
			}
		}
	}
	
	//释放内存
	FreeImage_Unload(dst);
	FreeImage_Unload(check);
	return vector;
}

//copy to vector
MatrixXd copyToVectorF(FIBITMAP *dib,int margin)//decodeJpeg
{
	//获取宽度，高度以及pitch
	unsigned int width = FreeImage_GetWidth(dib);
	if(width - 2*margin <= 0)
	{
		assert(width - 2*margin > 0);
		return MatrixXd::Zero(1,1);
	}
	unsigned int height = FreeImage_GetHeight(dib);
	unsigned int pitch = FreeImage_GetPitch(dib);
	//获取编码类型
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
	int x,y;
	int idx = 0;
	MatrixXd vector(1,height * (width - 2 * margin));
	if((image_type == FIT_BITMAP) && (FreeImage_GetBPP(dib) == 24)) 
	{
		//像素操作
		BYTE *bits = (BYTE*)FreeImage_GetBits(dib);
		for(x = margin; x < (int)width - margin; x++) 
		{
			for(y = (int)height - 1; y >= 0; y--) 
			{
				BYTE *pixel = bits + y*pitch + x*3; 
				int grayVal = (int)(0.30 * pixel[FI_RGBA_RED] + 0.59 * pixel[FI_RGBA_GREEN] + 0.11*pixel[FI_RGBA_BLUE]);
				double dVal = (double)grayVal / (double)255;
				vector(0,idx++) = dVal;
			}
		}
	}
	return vector;
}

//加载数据
bool loadDataSet(char *szFileName,MatrixXd &dataSet,MatrixXi &labels)
{
	ifstream ifs(szFileName);
	int dataDim = 1;
	int dataSetSize = 1;
	int i,j;
	if(ifs)
	{
		ifs >> dataDim >> dataSetSize;
		dataSet.resize(dataSetSize,dataDim);
		labels.resize(dataSetSize,1);
		for(i = 0; i < dataSetSize; i++)
		{
			for(j = 0; j < dataDim; j++)
			{
				ifs >> dataSet(i,j);
			}
		}
		for(i = 0; i < dataSetSize; i++)
		{
			ifs >> labels(i,0);
		}
		ifs.close();
		return true;
	}
	return false;
}

//保存数据
bool saveDataSet(char *szFileName,MatrixXd &dataSet,MatrixXi &labels,int dataSetSize)
{
	ofstream ofs(szFileName);
	int dataDim = dataSet.cols();
	if(ofs)
	{
		ofs << dataDim << " " << dataSetSize << endl;
		int i,j;
		for(i = 0; i < dataSetSize; i++)
		{
			for(j = 0; j < dataDim; j++)
			{
				ofs << dataSet(i,j) << " ";
			}
			ofs << endl;
		}
		ofs << endl;
		for(i = 0; i < dataSetSize; i++)
		{
			ofs << labels(i,0) << " ";
		}
		ofs.close();
		return true;
	}
	return false;
}

//获取一帧图片的卷积值，图片存储方式为列优先排列到向量中，filter与图片相同
MatrixXd getConvFeature(MatrixXd &vector,MatrixXd &filter,int imgWidth,int filterWidth,int convStep)
{
	int numConvFeatures = (imgWidth - filterWidth)/convStep + 1;
	MatrixXd convVal(1,numConvFeatures);
	int imgHeight = vector.cols() / imgWidth;
	int filterHeight = filter.cols() / filterWidth;
	assert(vector.maxCoeff() < 1);
	assert(vector.minCoeff() > 0);
	//cout << filterHeight << endl;
	//cout << "max:" << filter.maxCoeff() << endl;
	//do conv for numConvFeatures blocks
	for(int k = 0; k < numConvFeatures; k++)
	{
		double sum = 0;
		//calc convalution
		for(int i = 0; i <  filterHeight; i++)
		{
			MatrixXd m1 = filter.middleCols(i*filterWidth,filterWidth);
			MatrixXd m2 = vector.middleCols(i*imgWidth + k * convStep,filterWidth);
			MatrixXd val = m1 * m2.transpose();
	
			double v = val(0,0);
			
			assert(v < 10);
			assert(v > -10);
			
			sum += v;
		}
		convVal(0,k) = sum;
	}
	return convVal;
}


//获取一行conv feature 中的最大值，max-pooling
PoolStruct getPooledFeature(MatrixXd &convVal)
{
	double max = INT_MIN;
	int id = 0;
	for(int i = 0; i < convVal.cols(); i++)
	{
		if(convVal(0,i) > max)
		{
			max = convVal(0,i);
			id = i;
		}
	}
	PoolStruct ps;
	ps.val = max;
	ps.id = id;
	return ps;
}

bool loadTheta(MatrixXd theta1,MatrixXd theta2)
{
	ifstream ifs("theta.txt");
	//Error processing
	if(!ifs)
	{
		return false;
	}
	//Load theta1 from file
	int i,j;
	for(i = 0; i < theta1.rows(); i++)
	{
		for(j = 0; j < theta1.cols(); j++)
		{
			if(ifs.eof())
			{
				ifs.close();
				return false;
			}
			ifs >> theta1(i,j);
		}
	}
	//load theta2 from file
	for(i = 0; i < theta2.rows(); i++)
	{
		for(j = 0; j < theta2.cols(); j++)
		{
			if(ifs.eof())
			{
				ifs.close();
				return false;
			}
			ifs >> theta2(i,j);
		}
	}
	ifs.close();
	return true;
}

bool saveTheta(MatrixXd &theta1,MatrixXd &theta2)
{
	ofstream ofs("theta.txt");
	//Error processing
	if(!ofs)
	{
		return false;
	}
	//Save theta1 to file
	int i,j;
	for(i = 0; i < theta1.rows(); i++)
	{
		for(j = 0; j < theta1.cols(); j++)
		{
			ofs << theta1(i,j) << " ";
		}
		ofs << endl;
	}
	ofs << endl;
	//Save theta2 to file
	for(i = 0; i < theta2.rows(); i++)
	{
		for(j = 0; j < theta2.cols(); j++)
		{
			ofs << theta2(i,j) << " ";
		}
		ofs << endl;
	}
	ofs.close();
	return true;
}

//remove margin of every image
MatrixXd getNoMarginDataSet(MatrixXd &dataSet,int imgWidth,int margin)
{
	int width = imgWidth - 2 * margin;
	int length = dataSet.cols();
	//addLog(length);
	int t = length / imgWidth;
	assert(t > 0);
	if(t <= 0)
	{
		return MatrixXd::Zero(1,1);
	}
	//addLog(t);
	//MatrixXd dstDataSet = dataSet.middleCols(margin,width);
	//return dstDataSet;
	MatrixXd dstDataSet(dataSet.rows(),t*width);
	for(int i = 0; i < dataSet.rows(); i++)
	{
		for(int j = 0; j < t;j ++)
		{
			for(int k = 0; k < width;k++)
			{
				dstDataSet(i,j * width + k) = dataSet(i,j * imgWidth + k + margin);
			}
		}
	}
	return dstDataSet;
}

//保存矩阵数据
bool saveMat(MatrixXd &m,char *szFileName)
{
	ofstream ofs(szFileName);
	if(!ofs)
	{
		return false;
	}
	ofs << m.rows() << " " << m.cols() << endl;
	for(int i = 0; i < m.rows(); i++)
	{
		for(int j = 0; j < m.cols(); j++)
		{
			ofs << m(i,j) << endl;
		}
	}
	return true;
}

//加载矩阵数据
bool loadMat(MatrixXd &m,char *szFileName)
{
	ifstream ifs(szFileName);
	if(!ifs)
	{
		return false;
	}
	int rows,cols;
	ifs >> rows >> cols;
	m.resize(rows,cols);
	for(int i = 0; i < m.rows(); i++)
	{
		for(int j = 0; j < m.cols(); j++)
		{
			ifs >> m(i,j);
			if(ifs.eof())
			{
				ifs.close();
				return false;
			}
		}
	}
	return true;
}

MatrixXd getConvPooledDataSet(MatrixXd &dataSet,MatrixXd &filters,int imgWidth,int filterWidth,int convStep)
{
	//addLog(dataSet.rows());
	MatrixXd ret(dataSet.rows(),filters.rows());
	int filterNum = filters.rows();
	
	for(int i = 0; i <  filterNum; i++)
	{
		MatrixXd filter = filters.middleRows(i,1);
		for(int j = 0; j <  dataSet.rows(); j++)
		{
			MatrixXd vector = dataSet.middleRows(j,1);
			MatrixXd convFeature = getConvFeature(vector,filter,imgWidth,filterWidth,convStep);
			PoolStruct ps = getPooledFeature(convFeature);
			ret(j,i) = ps.val;
		}
	}
	return ret;
}

MatrixXd normalizeFeature(MatrixXd &m)
{
	MatrixXd min = m.rowwise().minCoeff();
	MatrixXd subMin = m - min.replicate(1,m.cols());
	MatrixXd sum = subMin.rowwise().sum();
	MatrixXd normalizedFeature = subMin.cwiseQuotient(sum.replicate(1,m.cols()));
	return normalizedFeature;
}

int predict(StackedAE &stackedAE,MatrixXd &vector)
{
	MatrixXd v = vector.transpose();
	int val = 0;
	MatrixXi pred = stackedAE.predict(v);
	val = pred(0,0);
	return val;
}



PredStruct inline predict(SAE &sae,Softmax &softmax,MatrixXd &vector,int imgWidth,int margin,int convStep)
{
	int filterWidth = imgWidth - 2 * margin;
	MatrixXd filters = sae.getTheta();
	int filterNum = filters.rows();
	assert(filterNum > 0);
	if(filterNum <= 0)
	{
		PredStruct ps1= {0,0};
		return ps1;
	}
	MatrixXd ret = MatrixXd::Zero(1,filterNum);
	int sum = 0;
	for(int i = 0; i <  filterNum; i++)
	{
		MatrixXd filter = filters.middleRows(i,1);
		MatrixXd convFeature = getConvFeature(vector,filter,imgWidth,filterWidth,convStep);
		PoolStruct ps = getPooledFeature(convFeature);
		ret(0,i) = ps.val;
		sum += ps.id;
	}
	PredStruct predstruct = {0,0};
	/*TCHAR str[200];
	_stprintf_s(str,_T("%d %d"),ret.rows(),ret.cols());
	AfxMessageBox(str);*/

	/*MatrixXd predData = normalizeFeature(ret).transpose();
	MatrixXi pred = softmax.predict(predData);*/
	MatrixXd predData = ret.transpose(); 
	MatrixXi pred = softmax.predict(predData);
	predstruct.val = pred(0,0);
	predstruct.relativePos = sum / filterNum;
	return predstruct;
}

MatrixXd inline predictV(SAE &sae,Softmax &softmax,MatrixXd &vector,int imgWidth,int margin,int convStep)
{
	int filterWidth = imgWidth - 2 * margin;
	MatrixXd filters = sae.getTheta();
	int filterNum = filters.rows();
	assert(filterNum > 0);
	if(filterNum <= 0)
	{
		return MatrixXd::Zero(1,1);
	}
	MatrixXd ret = MatrixXd::Zero(1,filterNum);
	int sum = 0;
	for(int i = 0; i <  filterNum; i++)
	{
		MatrixXd filter = filters.middleRows(i,1);
		MatrixXd convFeature = getConvFeature(vector,filter,imgWidth,filterWidth,convStep);
		PoolStruct ps = getPooledFeature(convFeature);
		ret(0,i) = ps.val;
		sum += ps.id;
	}
	return ret;
	//return normalizeFeature(ret);
}