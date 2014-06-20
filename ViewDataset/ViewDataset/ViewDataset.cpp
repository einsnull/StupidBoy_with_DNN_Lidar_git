#include "dataAndImage.h"

int main()
{
	MatrixXd data(1,1);
	MatrixXi labels(1,1);
	if(loadDataSet("data.txt",data,labels))
	{
		buildImage(data,48,"data.jpg");
	}
	return 0;
}