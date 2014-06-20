
// stupidBoyDlg.h : 头文件
//

#pragma once
#include "resource.h"
#include "tcpClient.h"
#include "DList.h"
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

class RecycleItem{
public:
	MatrixXd vector;
	int label;
	int bufId;
};

#define WM_UPDATENETVIEW WM_USER+1
#define WM_UPDATEFRAMERATE WM_USER+2
#define WM_ADDLISTSTR WM_USER+3
#define WM_UPDATEBUFFERCNT WM_USER+4
#define WM_LOADTHETA WM_USER+5

// CstupidBoyDlg 对话框
class CstupidBoyDlg : public CDialogEx
{
// 构造
public:
	//thread flag
	bool bViewThreadRelease;
	bool bCollectDataThreadRelease;
	bool bViewSensorThreadRelease;
	bool bDxInputThreadRelease;
	bool bManualDriveThreadRelease;
	bool bAutoDriveThreadRelease;
	bool bAutoDriveThread_F_Release;
	bool bNetViewThreadRelease;
	bool bUpdateSensorThreadRelease;
	//speed mode
	bool bSpeedHierarchy;
	int deltaHierarchy;

	//buffer param
	bool bDelBuf;
	int delBufNum;
	

	//Sensor state
	bool frontState;
	bool frontLeftState;
	bool frontRightState;
	bool leftState;
	bool rightState;
	bool frontObstacle;
	bool frontLeftObstacle;
	bool frontRightObstacle;
	bool leftObstacle;
	bool rightObstacle;
	CRITICAL_SECTION sensorCS;
	//
	int initNetSelected;
	int viewNetSelected;
	int netNum;
	//速度参数
	int speedThreshold;
	bool bTurningSupply;
	int speedSupply;
	int labelSupply;

	//图片参数
	//图片大小
	int imageWidth;
	int imageHeight;
	int margin;//图像左右两侧空余边缘
	int jpgBufSize;
	int dataBufferSize;
	//神经网络参数
	int inputLayerSize;
	int hiddenLayerSize;
	int hiddenLayerSize1;
	int hiddenLayerSize2;
	int outputLayerSize;
	int convStep;
	//数据集参数
	MatrixXd dataSet;
	MatrixXd labels;
	DList<RecycleItem> *pRecycleBin;
	int cntBufferedData;
	//网络参数
	int speed;
	int autoDriveSpeed;
	static char remoteAddr[101];
	int *dirTable;
	HDC hStaticViewDC;
	CDialog *pCViewDlg;
	int mode;//模式选择
	//ListBox
	int listCount;
	int dbgListCount;
	CDialog *pCListDlg;
	CRITICAL_SECTION dxInputCS;//dxInput更新互斥段
	int dxInputDirection; //dxInput方向
	bool bDxCSInitialization;
	bool bThetaCSInitialization;
	bool bStopDxInput;//停止dxInput标志
	bool bStopChildThread;
	bool bGrayMode;//显示模式标志
	bool bLoadTheta;//加载theta标志
	bool bScreenCapture;
	bool bStepDriveMode;
	bool bAngleShift;
	double displayRatio;
	TCPClient tcpDriveClient;
	HWND hMainDlg; //主窗口句柄
	HWND hDxInputSlider;//显示dxInput方向
	HWND hNetOutputSilder;//显示网络输出
	HWND hSpeedSlider;
	HWND hMsgList;//msg list
	HWND hDbgList;
	HANDLE hViewThread; 
	HANDLE hDxInputThread;
	HANDLE hChildThread;
	HINSTANCE hInstance;
	HANDLE hChildProcess;

	CstupidBoyDlg(CWnd* pParent = NULL);	// 标准构造函数
	//函数
	void addListStr(TCHAR *str);
// 对话框数据
	enum { IDD = IDD_STUPIDBOY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	int getDxInputDirection();

	void setDxInputDirection(int dir);

	void setDxInputSilder(int pos);

	void setNetOutputSlider(int pos);

	void setDxInputStatic(int label,int speed);

	int getLabel();

	int getSpeed();

	void updateNetView(int label);

	void updateFrameRate(int frameRate);

	void setDxSpeedSlider(int dxSpeed);

	void addDebugStr(TCHAR *str);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnIdok();
	afx_msg void OnCbnSelendokComboMode();
	afx_msg LRESULT OnUpdateNetView(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnUpdateFrameRate(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnAddListStr(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnCntBuffer(WPARAM wParam,LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonGraymode();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonShutdown();
	afx_msg void OnBnClickedButtonDelFrames();
	afx_msg void OnBnClickedButtonSeparate();
	afx_msg void OnBnClickedButtonTurningSupply();
	afx_msg void OnBnClickedButtonScreenCapture();
	afx_msg void OnBnClickedButtonTrain();
	afx_msg void OnBnClickedButtonChangeNetFlag();
	afx_msg void OnBnClickedButtonSpeedHierarchy();
};
