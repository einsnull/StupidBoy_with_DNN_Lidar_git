//training data:nDim * nExamples
#include <windows.h>
#include <iostream>
#include "dataAndImage.h"
#include "StackedAE.h"
//#include "ca.h"
using namespace std;

void printToFile(char *szFileName,char *str)
{
	ofstream ofs(szFileName,ios::app);
	if(ofs)
	{
		ofs << str << endl;
	}
	ofs.close();
}

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

double calcDist(MatrixXi &m1,MatrixXi &m2)
{
	double sum = (m1 - m2).leftCols(1).array().abs().sum();
	return sum / (double)m1.rows();
}

double slackAccurancy(MatrixXi &m1,MatrixXi &m2)
{
	int cnt = 0;
	MatrixXi m = (m1 - m2).leftCols(1).array().abs();
	for(int i = 0; i < m.rows(); i++)
	{
		if(m(i,0) <= 1)
		{
			cnt ++;
		}
	}
	return (double)cnt / (double) m.rows();
}

int main()
{
	int ae1HiddenSize = 100;
	int ae2HiddenSize = 100;
	int numClasses = 13;
	int imgWidth = 48;
	double noiseRatio[2] = {0.3,0.3};
	double lambda[4] = {3e-3,3e-3,3e-3,1e-4};
	double alpha[4] = {0.2,0.2,0.2,0.2};
	double beta[2] = {3,3};
	double sparsityParam[2] = {0.1,0.1};
	int maxIter[4] = {100,100,100,200};
	int miniBatchSize[4] = {20,20,20,20};
	char szTrainData[200] = "data5000Right.txt";
	char szTestData[200] = "data1000Right.txt";
#ifdef _WINDOWS_
	//set eigen threads
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	Eigen::setNbThreads(info.dwNumberOfProcessors);
#endif

	
	
	MatrixXi testLabels(1,1);
	char *fileBuf  = new char[4096];
	bool ret = loadFileToBuf("ParamConfig.ini",fileBuf,4096);
	if(ret)
	{
		getConfigDoubleValue(fileBuf,"noiseRatio0:",noiseRatio[0]);
		getConfigDoubleValue(fileBuf,"noiseRatio1:",noiseRatio[1]);
		getConfigDoubleValue(fileBuf,"lambda0:",lambda[0]);
		getConfigDoubleValue(fileBuf,"lambda1:",lambda[1]);
		getConfigDoubleValue(fileBuf,"lambda2:",lambda[2]);
		getConfigDoubleValue(fileBuf,"lambda3:",lambda[3]);
		getConfigDoubleValue(fileBuf,"alpha0:",alpha[0]);
		getConfigDoubleValue(fileBuf,"alpha1:",alpha[1]);
		getConfigDoubleValue(fileBuf,"alpha2:",alpha[2]);
		getConfigDoubleValue(fileBuf,"alpha3:",alpha[3]);
		getConfigDoubleValue(fileBuf,"beta0:",beta[0]);
		getConfigDoubleValue(fileBuf,"beta1:",beta[1]);
		getConfigDoubleValue(fileBuf,"sparseParam0:",sparsityParam[0]);
		getConfigDoubleValue(fileBuf,"sparseParam1:",sparsityParam[1]);
		getConfigIntValue(fileBuf,"maxIter0:",maxIter[0]);
		getConfigIntValue(fileBuf,"maxIter1:",maxIter[1]);
		getConfigIntValue(fileBuf,"maxIter2:",maxIter[2]);
		getConfigIntValue(fileBuf,"maxIter3:",maxIter[3]);
		getConfigIntValue(fileBuf,"miniBatchSize0:",miniBatchSize[0]);
		getConfigIntValue(fileBuf,"miniBatchSize1:",miniBatchSize[1]);
		getConfigIntValue(fileBuf,"miniBatchSize2:",miniBatchSize[2]);
		getConfigIntValue(fileBuf,"miniBatchSize3:",miniBatchSize[3]);
		getConfigIntValue(fileBuf,"ae1HiddenSize:",ae1HiddenSize);
		getConfigIntValue(fileBuf,"ae2HiddenSize:",ae2HiddenSize);
		getConfigIntValue(fileBuf,"imgWidth:",imgWidth);
		getConfigStrValue(fileBuf,"trainDataFile:",szTrainData,200);
		getConfigStrValue(fileBuf,"testDataFile:",szTestData,200);
		getConfigIntValue(fileBuf,"numClasses:",numClasses);
		cout << "numClasses: " << numClasses << endl;
		cout << "lambda0: " << lambda[0] << endl;
		cout << "lambda1: " << lambda[1] << endl;
		cout << "lambda2: " << lambda[2] << endl;
		cout << "lambda3: " << lambda[3] << endl;
		cout << "alpha0: " << alpha[0] << endl;
		cout << "alpha1: " << alpha[1] << endl;
		cout << "alpha2: " << alpha[2] << endl;
		cout << "alpha3: " << alpha[3] << endl;
		/*cout << "beta0: " << beta[0] << endl;
		cout << "beta1: " << beta[1] << endl;
		cout << "sparseParam0: " << sparsityParam[0] << endl;
		cout << "sparseParam1: " << sparsityParam[1] << endl;*/
		cout << "maxIter0: " << maxIter[0] << endl;
		cout << "maxIter1: " << maxIter[1] << endl;
		cout << "maxIter2: " << maxIter[2] << endl;
		cout << "maxIter3: " << maxIter[3] << endl;
		cout << "miniBatchSize0: " << miniBatchSize[0] << endl;
		cout << "miniBatchSize1: " << miniBatchSize[1] << endl;
		cout << "miniBatchSize2: " << miniBatchSize[2] << endl;
		cout << "miniBatchSize3: " << miniBatchSize[3] << endl;
		cout << "ae1HiddenSize: " << ae1HiddenSize << endl;
		cout << "ae2HiddenSize: " << ae2HiddenSize << endl;
		cout << "imgWidth: " << imgWidth << endl;
		cout << "TrainDataFile: " << szTrainData << endl;
		cout << "TestDataFile: " << szTestData << endl;
	}
	delete []fileBuf;
	//timer
	clock_t start = clock();
	
	cout << "Loading training data..." << endl;
	MatrixXd trainT(1,1);
	MatrixXi labelsTmp(1,1);
	if(!loadDataSet(szTrainData,trainT,labelsTmp))
	{
		cout << "Error in loading file.." << endl;
		system("pause");
		return -1;
	}
	
	int repNum = 3;
	
	MatrixXd trainingData = trainT.transpose();
	MatrixXi trainingLabels = labelsTmp;
	/*MatrixXd trainingData = trainT.transpose().replicate(1,repNum);
	MatrixXi trainingLabels = labelsTmp.replicate(repNum,1);*/
	//cout << "Trainingdata rows: " << trainingData.rows() << endl;
	/*MatrixXd randNoise = (MatrixXd::Random(1,trainT.rows() * (repNum - 1)) * 0.08).replicate(trainingData.rows(),1);
	trainingData.leftCols(trainT.rows() * (repNum - 1)) += randNoise;
	MatrixXd showData = trainingData.leftCols(200).transpose();
	buildImage(showData,imgWidth,"data.jpg",false);
	trainingData = setBound(trainingData);*/
 
	
	StackedAE stackedAE(ae1HiddenSize,ae2HiddenSize,numClasses);
	stackedAE.preTrain(trainingData,trainingLabels,lambda,alpha,miniBatchSize,
		maxIter,DENOISING_AE,noiseRatio,beta,sparsityParam);
	//stackedAE.loadModel("StackedAE_Model.txt");
	cout << "Loading test data..." << endl;
	MatrixXd testT(1,1);
	if(!loadDataSet(szTestData,testT,testLabels))
	{
		return -1;
	}
	MatrixXd testData = testT.transpose();
	
	cout << "testData rows " << testData.rows() << endl;
	MatrixXi pred1 = stackedAE.predict(testData);
	
	double acc1 = stackedAE.calcAccurancy(testLabels,pred1);
	cout << "Accurancy on testset(before): " << acc1 * 100 << "%" << endl;
	double sAcc1 = slackAccurancy(testLabels,pred1);
	cout << "Slack accurancy on testset(before): " << sAcc1 * 100 << "%" << endl;
	char str[100];
	sprintf_s(str,"Accurancy on testset(before): %lf %%",acc1 * 100);
	printToFile("LogFile.txt",str);
	MatrixXi pred4 = stackedAE.predict(trainingData);
	double acc4 = stackedAE.calcAccurancy(trainingLabels,pred4);
	cout << "Accurancy on trainingset: " << acc4 * 100 << "%" << endl;
	cout << "Dist on trainingset before fine tuning: " << calcDist(pred4,trainingLabels) << endl;
	double sAcc4 = slackAccurancy(trainingLabels,pred4);
	cout << "Slack Accurancy on trainingset(before): " << sAcc4 * 100 << "%" << endl;
	sprintf_s(str,"Accurancy on trainingset(before): %lf %%",acc4 * 100);
	printToFile("LogFile.txt",str);
	MatrixXd aeTheta1 = stackedAE.getAe1Theta();
	MatrixXd aeTheta2 = stackedAE.getAe2Theta();
	MatrixXd filter = aeTheta2 * aeTheta1;
	buildImage(aeTheta1,imgWidth,"ae1Before.jpg",false);
	buildImage(filter,imgWidth,"ae2Before.jpg",false);
	cout << "Fine Tuning..." << endl;

	stackedAE.fineTune(trainingData,trainingLabels,lambda[3],
		alpha[3],maxIter[3],miniBatchSize[3]);
	
	MatrixXi pred2 = stackedAE.predict(testData);
	double acc2 = stackedAE.calcAccurancy(testLabels,pred2);
	cout << "Accurancy: " << acc2 * 100 << "%" << endl;
	double sAcc2 = slackAccurancy(testLabels,pred2);
	cout << "Slack accurancy on testset(after): " << sAcc2 * 100 << "%" << endl;
	sprintf_s(str,"Accurancy on testset(after): %lf %%",acc2 * 100);
	printToFile("LogFile.txt",str);
	MatrixXi pred3 = stackedAE.predict(trainingData);
	double acc3 = stackedAE.calcAccurancy(trainingLabels,pred3);
	cout << "Accurancy on trainingset after fine tuning: " << acc3 * 100 << "%" << endl;
	cout << "Dist on trainingset after fine tuning: " << calcDist(pred3,trainingLabels) << endl;
	double sAcc3 = slackAccurancy(pred3,trainingLabels);
	cout << "Slack accurancy on trainingset(after): " << sAcc3 * 100 << "%" << endl;
	sprintf_s(str,"Accurancy on trainingset(after): %lf %%",acc3 * 100);
	printToFile("LogFile.txt",str);
	stackedAE.saveModel("StackedAE_Model.txt");
	aeTheta1 = stackedAE.getAe1Theta();
	aeTheta2 = stackedAE.getAe2Theta();
	filter = aeTheta2 * aeTheta1;
	buildImage(aeTheta1,imgWidth,"ae1After.jpg",false);
	buildImage(filter,imgWidth,"ae2After.jpg",false);
	clock_t end = clock();
	cout << "The code ran for " << 
		(end - start)/(double)(CLOCKS_PER_SEC*60) <<
		" minutes on " << Eigen::nbThreads() << " thread(s)." << endl;
	system("pause");
	return 0;
}

