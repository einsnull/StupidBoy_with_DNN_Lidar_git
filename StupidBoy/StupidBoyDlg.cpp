// stupidBoyDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "stupidBoy.h"
#include "stupidBoyDlg.h"
#include "afxdialogex.h"
#include "threads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




char CstupidBoyDlg::remoteAddr[101] = "127.0.0.1";
//int CstupidBoyDlg::dirTable[13] = {50,57,64,71,77,83,90,96,102,109,116,123,130};
//int CstupidBoyDlg::dirTable[7] = {50,64,77,90,102,116,130};
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CstupidBoyDlg �Ի���



CstupidBoyDlg::CstupidBoyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CstupidBoyDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CstupidBoyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CstupidBoyDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_COMMAND(IDOK, &CstupidBoyDlg::OnIdok)
	ON_CBN_SELENDOK(IDC_COMBO_MODE, &CstupidBoyDlg::OnCbnSelendokComboMode)
	ON_MESSAGE(WM_UPDATENETVIEW,OnUpdateNetView)
	ON_MESSAGE(WM_UPDATEFRAMERATE,OnUpdateFrameRate)
	ON_MESSAGE(WM_ADDLISTSTR,OnAddListStr)
	ON_MESSAGE(WM_UPDATEBUFFERCNT,OnCntBuffer)
	ON_BN_CLICKED(IDC_BUTTON_GRAYMODE, &CstupidBoyDlg::OnBnClickedButtonGraymode)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CstupidBoyDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SHUTDOWN, &CstupidBoyDlg::OnBnClickedButtonShutdown)
	ON_BN_CLICKED(IDC_BUTTON_DEL_FRAMES, &CstupidBoyDlg::OnBnClickedButtonDelFrames)
	ON_BN_CLICKED(IDC_BUTTON_SEPARATE, &CstupidBoyDlg::OnBnClickedButtonSeparate)
	ON_BN_CLICKED(IDC_BUTTON_TURNING_SUPPLY, &CstupidBoyDlg::OnBnClickedButtonTurningSupply)
	ON_BN_CLICKED(IDC_BUTTON_SCREEN_CAPTURE, &CstupidBoyDlg::OnBnClickedButtonScreenCapture)
	ON_BN_CLICKED(IDC_BUTTON_TRAIN, &CstupidBoyDlg::OnBnClickedButtonTrain)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_NET_FLAG, &CstupidBoyDlg::OnBnClickedButtonChangeNetFlag)
	ON_BN_CLICKED(IDC_BUTTON_SPEED_HIERARCHY, &CstupidBoyDlg::OnBnClickedButtonSpeedHierarchy)
END_MESSAGE_MAP()


// CstupidBoyDlg ��Ϣ�������

BOOL CstupidBoyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//thread flags

	bViewThreadRelease = true;
	bCollectDataThreadRelease = true;
	bViewSensorThreadRelease = true;
	bDxInputThreadRelease = true;
	bManualDriveThreadRelease = true;
	bAutoDriveThreadRelease = true;
	bAutoDriveThread_F_Release = true;
	bNetViewThreadRelease = true;
	bUpdateSensorThreadRelease = true;
	//speed mode
	bSpeedHierarchy = false;
	deltaHierarchy = 0;
	//buffer param
	bDelBuf = false;
	
	//�������л�����
	netNum = 2;
	initNetSelected = 1;
	
	//ͼƬ���� 
	bScreenCapture = false;
	margin = 3;
	imageWidth = 48;
	imageHeight = 30;
	jpgBufSize = 10*1024*1024;
	//��ʼ�� bDxCriticalSection ��־
	bDxCSInitialization = false;

	//��ʼ�����
	hViewThread = NULL;
	hDxInputThread = NULL;

	//��ȡʵ�����
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainDlg = pApp -> m_pMainWnd;
	hMainDlg = pMainDlg -> m_hWnd; //ʵ�����������
	hInstance = pApp -> m_hInstance; //ʵ�����	
	
	int fileBufSize = 40 * 1024;
	char *fileBuf = new char[fileBufSize];
	bool ret = loadFileToBuf("ParamConfig.ini",fileBuf,fileBufSize);
	//������ʼ��
	int range = 7;
	outputLayerSize = range;
	speedThreshold = 10;
	labelSupply = 2;
	speedSupply = 10;
	convStep = 3;
	hiddenLayerSize1 = 20;
	hiddenLayerSize2 = 20;
	bool bMaped = false;
	if(ret)
	{
		getConfigIntValue(fileBuf,"hls1:",hiddenLayerSize1);
		getConfigIntValue(fileBuf,"hls2:",hiddenLayerSize2);
		getConfigIntValue(fileBuf,"outputLayerSize:",range);
		getConfigStrValue(fileBuf,"remoteAddr:",remoteAddr,100);
		getConfigIntValue(fileBuf,"speedThreshold:",speedThreshold);
		getConfigIntValue(fileBuf,"hiddenLayerSize:",hiddenLayerSize);
		getConfigIntValue(fileBuf,"outputLayerSize:",outputLayerSize);
		dirTable = new int[outputLayerSize];
		getConfigIntValue(fileBuf,"margin:",margin);
		getConfigIntValue(fileBuf,"imageWidth:",imageWidth);
		getConfigIntValue(fileBuf,"imageHeight:",imageHeight);
		getConfigIntValue(fileBuf,"speedSupply:",speedSupply);
		getConfigIntValue(fileBuf,"labelSupply:",labelSupply);
		getConfigIntValue(fileBuf,"dataSetSize:",dataBufferSize);
		bMaped = getConfigMapTable(fileBuf,"dirMapTable:",dirTable,outputLayerSize);
		getConfigIntValue(fileBuf,"autoDriveSpeed:",autoDriveSpeed);
		getConfigIntValue(fileBuf,"convStep:",convStep);
		getConfigIntValue(fileBuf,"init:",initNetSelected);
		getConfigIntValue(fileBuf,"netNum:",netNum);
		getConfigIntValue(fileBuf,"deltaHierarchy:",deltaHierarchy);
		
	}
	else
	{
		AfxMessageBox(_T("Couldn't find Config File!"));
		delete []fileBuf;
		ExitProcess(0);
	}
	delete []fileBuf;
	if(!bMaped)
	{
		AfxMessageBox(_T("Couldn't load dirMapTabel!"));
		ExitProcess(0);
	}
	viewNetSelected = initNetSelected;
	
	int exampleWth = imageWidth - 2*margin;
	inputLayerSize = exampleWth * imageHeight;
	//dataBufferSize = 1000;
	pRecycleBin = new DList<RecycleItem>(dataBufferSize);
	
	//��ʼ��dxInput������ʾ��
	CSliderCtrl *pDxSlider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_DXINPUT);
	pDxSlider->SetRange(0,range-1);
	pDxSlider->SetSelection(0,range-1);
	hDxInputSlider = pDxSlider->m_hWnd;

	//��ʼ���������������ʾ��
	CSliderCtrl *pNetOutSlider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_NETOUTPUT);
	pNetOutSlider->SetRange(0,range-1);
	pNetOutSlider->SetSelection(0,range-1);
	hNetOutputSilder = pNetOutSlider->m_hWnd;

	//��ʼ���ٶȻ�����ʾ��
	CSliderCtrl *pSpeedSlider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_SPEED);

	pSpeedSlider->SetRange(0,200 - speedThreshold);
	pSpeedSlider->SetSelection(0,200 - speedThreshold);
	hSpeedSlider = pSpeedSlider->m_hWnd;

	//������ͼ���ڲ���ȡ��ͼ����DC
	displayRatio = 1;
	pCViewDlg = new CDialog;
	pCViewDlg->Create(IDD_DIALOG_VIEW);
	pCViewDlg->ShowWindow(SW_SHOW);
	hStaticViewDC = pCViewDlg->GetDlgItem(IDC_STATIC_VIEW)->GetDC()->m_hDC;


	//����list����
	listCount = 0;
	pCListDlg = new CDialog;
	pCListDlg->Create(IDD_DIALOG_LIST);
	pCListDlg->ShowWindow(SW_SHOW);
	hMsgList = pCListDlg->GetDlgItem(IDC_LIST)->m_hWnd;
	dbgListCount = 0;
	hDbgList = GetDlgItem(IDC_DEBUG_LIST)->m_hWnd;

	//������ͼ�����߳�
	bGrayMode = false;

	//����dxInput�߳�
	bStopDxInput = false;
	hDxInputThread = CreateThread(NULL,0,dxInputThread,this,0,0);

	//��ʾ��Ϣ
	addListStr(_T("Welcome"));

	//��ʼ���ٶ�ֵ
	speed = 20;
	hChildThread = NULL;
	bLoadTheta = false;
	bStepDriveMode = false;
	cntBufferedData = 0;
	bTurningSupply = false;
	bAngleShift = true;//�����Ƕ�Ư�����ݼ�����

	SetDlgItemInt(IDC_EDIT_DEL_FRAMES,0);
	OnBnClickedButtonSeparate();
	
	frontState = true;
	frontLeftState = true;
	frontRightState = true;
	leftState = true;
	rightState = true;
	frontObstacle = true;
	frontLeftObstacle = true;
	frontRightObstacle = true;
	leftObstacle = true;
	rightObstacle = true;
	InitializeCriticalSection(&sensorCS);
	HANDLE hThread = CreateThread(NULL,0,updateSensorThread,this,0,0);
	CloseHandle(hThread);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CstupidBoyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CstupidBoyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CstupidBoyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CstupidBoyDlg::DestroyWindow()
{
	// TODO: �ڴ����ר�ô����/����û���
	delete pCViewDlg; //������ͼ����
	delete pCListDlg; //����list����
	return CDialogEx::DestroyWindow();
}


void CstupidBoyDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	OnBnClickedButtonStop();
	bStopChildThread = true;
	((CComboBox *)GetDlgItem(IDC_COMBO_MODE))->SetCurSel(4);
	bStopDxInput = true;
	//
	//while(!(bViewThreadRelease && bViewSensorThreadRelease
	//	&& bCollectDataThreadRelease 
	//	&& bManualDriveThreadRelease && bAutoDriveThreadRelease
	//	&& bAutoDriveThread_F_Release && bNetViewThreadRelease))
	//{
	//	SwitchToThread();
	//}
	//
	HANDLE h = CreateThread(NULL,0,waitThread,this,0,0);
	WaitForSingleObject(h,2000);
	CloseHandle(h);

	
	CDialogEx::OnClose();
}


void CstupidBoyDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	//����dxInput�����
	if(bDxCSInitialization)
	{
		DeleteCriticalSection(&dxInputCS);
	}
	DeleteCriticalSection(&sensorCS);
}


void CstupidBoyDlg::OnIdok()
{
	// TODO: �ڴ���������������
}


void CstupidBoyDlg::addListStr(TCHAR *str)
{
	if(listCount >= 19)
	{
		listCount --;
		::SendMessage(hMsgList,LB_DELETESTRING,(WPARAM)0,0);
	}

	::SendMessage(hMsgList,LB_INSERTSTRING,(WPARAM)listCount,(LPARAM)str);
	listCount ++;
}

void CstupidBoyDlg::OnCbnSelendokComboMode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	mode = ((CComboBox *)GetDlgItem(IDC_COMBO_MODE))->GetCurSel();
	HANDLE hThread = CreateThread(NULL,0,selectModeThread,this,0,0);
	CloseHandle(hThread);

}

//��ȡdxInput����
int CstupidBoyDlg::getDxInputDirection()
{
	int d = 0;
	if(!bDxCSInitialization)
	{
		bDxCSInitialization = true;
		InitializeCriticalSection(&dxInputCS);
	}
	EnterCriticalSection(&dxInputCS);
	d = dxInputDirection;
	LeaveCriticalSection(&dxInputCS);
	return d;
}

//����dxInput����
void CstupidBoyDlg::setDxInputDirection(int dir)
{
	if(!bDxCSInitialization)
	{
		bDxCSInitialization = true;
		InitializeCriticalSection(&dxInputCS);
	}
	EnterCriticalSection(&dxInputCS);
	dxInputDirection = dir;
	LeaveCriticalSection(&dxInputCS);
}

int CstupidBoyDlg::getLabel()
{
	return GetDlgItemInt(IDC_STATIC_DXDIR);
}

int CstupidBoyDlg::getSpeed()
{
	int speed = GetDlgItemInt(IDC_STATIC_SPEED);
	return speed;
}


BOOL CstupidBoyDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if(pMsg->wParam == WM_KEYDOWN)
	{
		return true;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CstupidBoyDlg::setDxInputSilder(int pos)
{
	::SendMessage(hDxInputSlider,TBM_SETPOS, (WPARAM)TRUE,pos);
}

void CstupidBoyDlg::setNetOutputSlider(int pos)
{
	::SendMessage(hNetOutputSilder,TBM_SETPOS, (WPARAM)TRUE,pos);
}

LRESULT CstupidBoyDlg::OnUpdateNetView(WPARAM wParam,LPARAM lParam)
{
	int pos = (int)wParam;
	setNetOutputSlider(pos);
	TCHAR str[255];
	wsprintf(str,_T("%d"),pos);
	addListStr(str);
	return 0;
}

LRESULT CstupidBoyDlg::OnUpdateFrameRate(WPARAM wParam,LPARAM lParam)
{
	SetDlgItemInt(IDC_STATIC_FRAMERATE,(int)wParam);
	return 0;
}

LRESULT CstupidBoyDlg::OnAddListStr(WPARAM wParam,LPARAM lParam)
{
	int len = 7;
	TCHAR *str[7] = {_T("Loading Matlab Dll"),_T("Matlab Dll loaded"),
		_T("init Matlab Dll failed"),_T("Open File Failed"),
		_T("StupidBoy not found."),_T("Child Process Exit."),
		_T("Training...")};
	int id = (int)wParam;
	if(id >= len)
	{
		return -1;
	}
	addListStr(str[id]);
	return 0;
}



void CstupidBoyDlg::setDxInputStatic(int label,int speed)
{
	if(bStepDriveMode == true)
	{
		if(speed > speedThreshold)
		{
			speed = speedThreshold+5;
		}
	}
	SetDlgItemInt(IDC_STATIC_SPEED,speed);
	SetDlgItemInt(IDC_STATIC_DXDIR,label);
}

LRESULT CstupidBoyDlg::OnCntBuffer(WPARAM wParam,LPARAM lParam)
{
	SetDlgItemInt(IDC_STATIC_BUFFEREDFRAMES,(int)wParam);
	return 0;
}

void CstupidBoyDlg::OnBnClickedButtonGraymode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(bGrayMode == false)
	{
		SetDlgItemText(IDC_BUTTON_GRAYMODE,_T("GrayModeOff"));
		bGrayMode = true;
	}
	else
	{
		SetDlgItemText(IDC_BUTTON_GRAYMODE,_T("GrayMode"));
		bGrayMode = false;
	}
}


void CstupidBoyDlg::updateNetView(int label)
{
	setNetOutputSlider(label);
}

void CstupidBoyDlg::updateFrameRate(int frameRate)
{
	SetDlgItemInt(IDC_STATIC_FRAMERATE,frameRate);
}

void CstupidBoyDlg::OnBnClickedButtonStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	bStopChildThread = true;
	((CComboBox *)GetDlgItem(IDC_COMBO_MODE))->SetCurSel(4);
	SwitchToThread();
}


void CstupidBoyDlg::OnBnClickedButtonShutdown()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	bStopChildThread = true;
	//SwitchToThread();
	TCPClient client;
	char *str = "0 255";
	if(client.connectHost(remoteAddr,2013))
	{
		client.sendData(str,(int)strlen(str));
		client.disconnect();
	}
}


void CstupidBoyDlg::setDxSpeedSlider(int dxSpeed)
{
	::SendMessage(hSpeedSlider,TBM_SETPOS, (WPARAM)TRUE,(LPARAM)dxSpeed);
}

void CstupidBoyDlg::OnBnClickedButtonDelFrames()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int num = GetDlgItemInt(IDC_EDIT_DEL_FRAMES);
	//delBuffer(*pDataSet,*pLabels,dataSetSize,cntBufferedData,num,*pRecycleBin);
	//delBuf(cntBufferedData,num);
	delBufNum = num;
	bDelBuf = true;
	SetDlgItemInt(IDC_EDIT_DEL_FRAMES,0);
	SetDlgItemInt(IDC_STATIC_BUFFEREDFRAMES,cntBufferedData);
}

void CstupidBoyDlg::OnBnClickedButtonSeparate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString str;
	if(GetDlgItemText(IDC_BUTTON_SEPARATE, str), str == _T("<<"))
	{
		SetDlgItemText(IDC_BUTTON_SEPARATE,_T(">>"));
	}
	else
	{
		SetDlgItemText(IDC_BUTTON_SEPARATE,_T("<<"));
	}

	static CRect rectLarge;
	static CRect rectSmall;
	
	if(rectLarge.IsRectNull())
	{
		CRect rectSeparator;
		GetWindowRect(&rectLarge);
		GetDlgItem(IDC_SEPARATOR)->GetWindowRect(&rectSeparator);

		rectSmall.left=rectLarge.left;
		rectSmall.top=rectLarge.top;
		rectSmall.right=rectLarge.right;
		rectSmall.bottom=rectSeparator.bottom;
	}
	if(str==_T("<<"))
	{
		SetWindowPos(NULL,0,0,rectSmall.Width(),rectSmall.Height(),
			SWP_NOMOVE | SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(NULL,0,0,rectLarge.Width(),rectLarge.Height(),
			SWP_NOMOVE | SWP_NOZORDER);
	}
}


void CstupidBoyDlg::OnBnClickedButtonTurningSupply()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(bTurningSupply == true)
	{
		bTurningSupply = false;
		SetDlgItemText(IDC_BUTTON_TURNING_SUPPLY,_T("Turning Supply"));
	}
	else
	{
		bTurningSupply = true;
		SetDlgItemText(IDC_BUTTON_TURNING_SUPPLY,_T("T-Supply Off"));
	}
}

void CstupidBoyDlg::addDebugStr(TCHAR *str)
{
	if(dbgListCount >= 5)
	{
		dbgListCount --;
		::SendMessage(hDbgList,LB_DELETESTRING,(WPARAM)0,0);
	}

	::SendMessage(hDbgList,LB_INSERTSTRING,(WPARAM)dbgListCount,(LPARAM)str);
	dbgListCount ++;
}



void CstupidBoyDlg::OnBnClickedButtonScreenCapture()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	bScreenCapture = true;
}


void CstupidBoyDlg::OnBnClickedButtonTrain()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HANDLE hThread = CreateThread(NULL,0,trainThread,this,0,0);
	CloseHandle(hThread);
}


void CstupidBoyDlg::OnBnClickedButtonChangeNetFlag()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	viewNetSelected ++;
	viewNetSelected %= netNum;
	/*if(viewNetSelected >= netNum)
	{
		viewNetSelected = 0;
	}*/
}


void CstupidBoyDlg::OnBnClickedButtonSpeedHierarchy()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(bSpeedHierarchy == true)
	{
		bSpeedHierarchy = false;
		SetDlgItemText(IDC_BUTTON_SPEED_HIERARCHY,_T("SpeedHierarchy"));
	}
	else
	{
		bSpeedHierarchy = true;
		SetDlgItemText(IDC_BUTTON_SPEED_HIERARCHY,_T("sHierarchyOff"));
	}
}
