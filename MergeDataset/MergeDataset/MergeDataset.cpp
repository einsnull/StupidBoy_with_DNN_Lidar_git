#include "GetConfig.h"
#include <Eigen/Dense>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Eigen;

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

//±£´æÊý¾Ý
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

int main()
{
	MatrixXd data1(1,1);
	MatrixXd data2(1,1);
	MatrixXi labels1(1,1);
	MatrixXi labels2(1,1);
	char szData1[255];
	char szData2[255];
	char szDstDataFile[255];
	char *fileBuf  = new char[4096];
	bool ret = loadFileToBuf("ParamConfig.ini",fileBuf,4096);
	if(ret)
	{
		getConfigStrValue(fileBuf,"dataFile1:",szData1,255);
		getConfigStrValue(fileBuf,"dataFile2:",szData2,255);
		getConfigStrValue(fileBuf,"dstDataFile:",szDstDataFile,255);
		cout << "dataFile1: " << szData1 << endl;
		cout << "dataFile2: " << szData2 << endl;
		cout << "dstDataFile: " << szDstDataFile << endl;
	}
	delete []fileBuf;

	if(!loadDataSet(szData1,data1,labels1))
	{
		cout << "Couldn't open dataFile1." << endl;
		return -1;
	}
	if(!loadDataSet(szData2,data2,labels2))
	{
		cout << "Couldn't open dataFile2." << endl;
		return -1;
	}

	MatrixXd data(data1.rows() + data2.rows(),data1.cols());
	MatrixXi labels(labels1.rows() + labels2.rows(),1);
	data.topRows(data1.rows()) = data1;
	data.bottomRows(data2.rows()) = data2;
	labels.topRows(labels1.rows()) = labels1;
	labels.bottomRows(labels2.rows()) = labels2;
	saveDataSet(szDstDataFile,data,labels,data.rows());
	cout << "Done." << endl;
	system("pause");
	return 0;
}