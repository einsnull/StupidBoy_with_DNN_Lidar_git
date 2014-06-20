#include <iostream>
#include <stdio.h>
#include "Softmax.h"
#include "GetConfig.h"

using namespace std;

double bound(double x)
{
	if(x > 1)
	{
		return 1;
	}
	if(x < 0)
	{
		return 0;
	}
	return x;
}

MatrixXd setBound(MatrixXd &m)
{
	return m.unaryExpr(ptr_fun(bound));
}

//¼ÓÔØÊý¾Ý
bool loadDataSet(char* szFileName,MatrixXd &dataSet,MatrixXi &labels)
{
	ifstream ifs(szFileName);
	int dataDim = 1;
	int dataSetSize = 1;
	int i,j;
	if(ifs)
	{
		ifs >> dataDim >> dataSetSize;
		cout << "dataDim: " << dataDim <<  " dataset size: " << dataSetSize << endl;
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

int main()
{

	double lambda = 1e-4;
	double alpha = 0.01;
	int maxIter = 3000;
	int miniBatchSize = 100;
	int numClasses = 2;

	cout << "Loading training data..." << endl;

	char szTrainData[200];
	int datasetNum = 2;
	int datasetSize = 1000;

	char *fileBuf  = new char[4096];
	bool ret = loadFileToBuf("ParamConfig.ini",fileBuf,4096);
	if(ret)
	{
		getConfigIntValue(fileBuf,"datasetNum:",datasetNum);
		getConfigIntValue(fileBuf,"datasetSize:",datasetSize);
		cout << "datasetNum: " << datasetNum << endl;
		cout << "datasetSize: " << datasetSize << endl;
	}
	delete []fileBuf;

	MatrixXd trainT(1,1);
	MatrixXi labelsTmp(1,1);
	MatrixXd trainingData(1440,datasetNum * datasetSize);
	MatrixXi trainingLabels(datasetNum * datasetSize,1);
	int n = 0;
	for(int i = 0; i < datasetNum; i++)
	{
		sprintf_s(szTrainData,"data_%d.txt",i);
		if(!loadDataSet(szTrainData,trainT,labelsTmp))
		{
			cout << "Error in loading file.." << endl;
			system("pause");
			return -1;
		}
		trainingData.middleCols(n,trainT.rows()) = trainT.transpose();
		trainingLabels.middleRows(n,trainT.rows()) = MatrixXi::Ones(trainT.rows(),1) * i;
		n = trainT.rows();
	}

	
	int inputSize = trainingData.rows();
	Softmax softmax(inputSize,numClasses);
	cout << "Training..." << endl;
	softmax.train(trainingData,trainingLabels,lambda,alpha,maxIter,miniBatchSize);

	//cout << "Loading test data..." << endl;
	
	softmax.saveModel("Softmax_Model.txt");
	system("pause");
	return 0;
}
