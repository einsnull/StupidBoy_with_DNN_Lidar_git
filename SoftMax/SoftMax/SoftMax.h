#pragma once
#include <Eigen/Dense>
#include <ctime>
#include <fstream>
#include <iostream>

using namespace std;
using namespace Eigen;

//trainingData  ndim * numOfExamples
//labels numOfExamples * 1

class Softmax
{
private:
	MatrixXd theta;
	int inputSize;
	int numClasses;
public:
	Softmax(int inputSize,int numClasses);
	MatrixXi predict(MatrixXd &data);
	void train(MatrixXd &data,MatrixXi &labels,
		double lambda,double alpha,
		int maxIter,int miniBatchSize);
	double calcAccurancy(MatrixXi &pred,MatrixXi &labels);
	bool saveModel(char *szFileName);
	bool loadModel(char *szFileName);
	MatrixXd getTheta();
private:
	MatrixXd binaryCols(MatrixXi &labels,int numOfClasses);
	MatrixXd expMat(MatrixXd &z);
	MatrixXd logMat(MatrixXd &z);
	MatrixXd randomInitialize(int lIn,int lOut);
	double computeCost(double lambda,MatrixXd &data,
		MatrixXi &labels,MatrixXd &thetaGrad);
	MatrixXd bsxfunMinus(MatrixXd &m,MatrixXd &x);
	MatrixXd bsxfunRDivide(MatrixXd &m,MatrixXd &x);
	void Softmax::miniBatchSGD(MatrixXd &trainData,MatrixXi &labels,
		double lambda,double alpha,int maxIter,int batchSize);
};

MatrixXd Softmax::bsxfunMinus(MatrixXd &m,MatrixXd &x)
{
	MatrixXd result = m;
	if(x.rows() == 1)
	{
		for(int i = 0;i < m.rows(); i++)
		{
			result.row(i) = m.row(i) - x;
		}
	}
	if(x.cols() == 1)
	{
		for(int i = 0;i < m.cols(); i++)
		{
			result.col(i) = m.col(i) - x;
		}
	}
	return result;
}

MatrixXd Softmax::bsxfunRDivide(MatrixXd &m,MatrixXd &x)
{
	MatrixXd result = m;
	if(x.rows() == 1)
	{
		for(int i = 0;i < m.rows(); i++)
		{
			result.row(i) = m.row(i).cwiseQuotient(x);
		}
	}
	if(x.cols() == 1)
	{
		for(int i = 0;i < m.cols(); i++)
		{
			result.col(i) = m.col(i).cwiseQuotient(x);
		}
	}
	return result;
}

Softmax::Softmax(int inputSize,int numClasses)
{
	this ->inputSize = inputSize;
	this ->numClasses = numClasses;
	theta = randomInitialize(numClasses,inputSize);
}

MatrixXd Softmax::randomInitialize(int lIn,int lOut)
{
	//random initialize the weight
	int i,j;
	double epsilon = sqrt(6.0/(lIn + lOut + 1));
	MatrixXd result(lIn,lOut);
	srand((unsigned int)time(NULL));
	for(i = 0;i < lOut;i++)
	{
		for(j = 0;j < lIn;j++)
		{
			result(j,i) = ((double)rand() / (double)RAND_MAX) * 2 * epsilon - epsilon; 
		}
	}
	return result;
}

//component wise log function
MatrixXd Softmax::logMat(MatrixXd &z)
{
	MatrixXd result(z.rows(),z.cols());
	for(int i = 0;i < z.rows();i++)
	{
		for(int j = 0;j < z.cols();j++)
		{
			result(i,j) = log(z(i,j));
		}
	}
	return result;
}

//component wise exp function
MatrixXd Softmax::expMat(MatrixXd &z)
{
	MatrixXd result(z.rows(),z.cols());
	for(int i = 0;i < z.rows();i++)
	{
		for(int j = 0;j < z.cols();j++)
		{
			result(i,j) = exp(z(i,j));
		}
	}
	return result;
}

//set targets to binary code
MatrixXd Softmax::binaryCols(MatrixXi &labels,int numOfClasses)
{
	// return binary code of labels
	//eye function
	MatrixXd e = MatrixXd::Identity(numOfClasses,numOfClasses);
	int numOfExamples = labels.rows();
	int inputSize = e.cols();
	MatrixXd result(inputSize,numOfExamples);
	for(int i = 0; i < numOfExamples; i++)
	{
		int idx = labels(i,0);
		result.col(i) = e.col(idx);
	}
	return result;
}

MatrixXi Softmax::predict(MatrixXd &data)
{
	//cout << theta.rows() << " " << theta.cols() << endl;
	//cout << data.rows() << " " << data.cols() << endl;
	MatrixXd m = theta * data;
	MatrixXi pred(m.cols(),1);
	for(int i = 0; i < m.cols(); i++)
	{
		double max = 0;
		int idx = 0;
		for(int j = 0; j < m.rows();j++)
		{
			if(m(j,i) > max)
			{
				max = m(j,i);
				idx = j;
			}
		}
		pred(i,0) = idx;
	}
	return pred;
}

double Softmax::computeCost(double lambda,MatrixXd &data,
							MatrixXi &labels,MatrixXd & thetaGrad)
{
	int numCases = data.cols();
	MatrixXd groundTruth = binaryCols(labels,numClasses);
	//cout << theta.rows() << " " << theta.cols() << endl;
	//cout << data.rows() << " " << data.cols() << endl;
	MatrixXd M = theta * data;
	MatrixXd maxM = M.colwise().maxCoeff();
	M = bsxfunMinus(M,maxM);
	MatrixXd expM = expMat(M);
	//cout << expM.rows() << " " << expM.cols() << endl;
	MatrixXd tmp1 = (expM.colwise().sum()).replicate(numClasses,1);
	//cout << tmp1.rows() << " " << tmp1.cols() << endl;
	MatrixXd p = expM.cwiseQuotient(tmp1);
	double cost = (groundTruth.cwiseProduct(logMat(p))).sum() * (-1.0 / numCases)
		+ (lambda / 2.0) * theta.array().square().sum();
	
	thetaGrad = (groundTruth - p) * data.transpose() * (-1.0 / numCases)
		+ theta * lambda;
	return cost;
}

//mini batch stochastic gradient descent
void Softmax::miniBatchSGD(MatrixXd &trainingData,MatrixXi &labels,double lambda,
						   double alpha,int maxIter,int batchSize)
{
	//get the binary code of labels
	MatrixXd thetaGrad(theta.rows(),theta.cols());
	MatrixXd miniTrainingData(trainingData.rows(),batchSize);
	MatrixXi miniLabels(batchSize,1);
	int iter = 1;
	int numBatches = trainingData.cols() / batchSize;
	
	//mini batch stochastic gradient decent
	for(int i = 0; i < maxIter;i++)
	{
		double J = 0;
		// compute the cost
		for(int j = 0;j < numBatches; j++)
		{
			miniTrainingData = trainingData.middleCols(j * batchSize,batchSize);
			miniLabels = labels.middleRows(j * batchSize,batchSize);
			J += computeCost(lambda,miniTrainingData,miniLabels,thetaGrad);
#ifdef _IOSTREAM_
			if(miniTrainingData.cols() < 1 || miniTrainingData.rows() < 1)
			{
				cout << "Too few training examples!"  << endl; 
			}
#endif

			if(fabs(J) < 0.001)
			{
				break;
			}
			//update theta
			theta -= thetaGrad * alpha;
		}
		J = J / numBatches;
#ifdef _IOSTREAM_
		cout << "iter: " << iter++ << "  cost: " << J << endl;
#endif
	}
}

void Softmax::train(MatrixXd &data,MatrixXi &labels,
					double lambda,double alpha,
					int maxIter,int miniBatchSize)
{
	miniBatchSGD(data,labels,lambda,alpha,maxIter,miniBatchSize);
}

double Softmax::calcAccurancy(MatrixXi &pred,MatrixXi &labels)
{
	int numOfExamples = pred.rows();
	double sum = 0;
	for(int i = 0; i < numOfExamples; i++)
	{
		if(pred(i,0) == labels(i,0))
		{
			sum += 1;
		}
	}
	return sum / numOfExamples;
}

//save model to file
bool Softmax::saveModel(char *szFileName)
{
	ofstream ofs(szFileName);
	if(!ofs)
	{
		return false;
	}
	int i,j;
	ofs << inputSize << " " << numClasses << endl;
	for(i = 0; i < theta.rows(); i++)
	{
		for(j = 0;j < theta.cols(); j++)
		{
			ofs << theta(i,j) << " ";
		}
	}
	ofs.close();
	return true;
}

//load model from file
bool Softmax::loadModel(char *szFileName)
{
	ifstream ifs(szFileName);
	if(!ifs)
	{
		return false;
	}
	ifs >> this -> inputSize >> this -> numClasses;
	int i,j;
	theta.resize(numClasses,inputSize);

	for(i = 0; i < theta.rows(); i++)
	{
		for(j = 0;j < theta.cols(); j++)
		{
			if(ifs.eof())
			{
				ifs.close();
				return false;
			}
			ifs >> theta(i,j);
		}
	}
	ifs.close();
	return true;
}

MatrixXd Softmax::getTheta()
{
	return theta;
}