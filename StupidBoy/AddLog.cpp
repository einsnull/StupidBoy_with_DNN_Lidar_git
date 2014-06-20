#include "stdAfx.h"
#include "AddLog.h"
void addLog(char *str)
{
	ofstream ofs("log.txt",ios::app);
	if(ofs)
	{
		ofs << str << endl;
		ofs.close();
	}
}

void addLog(int x)
{
	ofstream ofs("log.txt",ios::app);
	if(ofs)
	{
		ofs << x << endl;
		ofs.close();
	}
}

void addLog(double x)
{
	ofstream ofs("log.txt",ios::app);
	if(ofs)
	{
		ofs << x << endl;
		ofs.close();
	}
}