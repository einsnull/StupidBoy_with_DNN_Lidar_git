
// stupidBoy.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CstupidBoyApp:
// �йش����ʵ�֣������ stupidBoy.cpp
//

class CstupidBoyApp : public CWinApp
{
public:
	CstupidBoyApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CstupidBoyApp theApp;