#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>    
#include <tchar.h>


#pragma comment(lib, "dinput8.lib")
#pragma comment(lib,"dxguid.lib")

typedef struct{
	int keyId;
	int x;
	int y;
}CTRLS;


class DXINPUT
{
private:
	static DWORD dwFlags;
	static LPDIRECTINPUT8  g_pDirectInput;
	static LPDIRECTINPUTDEVICE8 g_pJoyStick;
	static DIJOYSTATE2  g_diJoyStickState;
	static HWND g_hWnd;
	static HINSTANCE g_hInstance;
	static LPDIRECTINPUTDEVICE8 g_pKeyboardDevice;  
	static char g_pKeyStateBuffer[256];
public:
	static bool bGameCtrl;
private:
	static BOOL FAR PASCAL enumDeviceCallBack(LPCDIDEVICEINSTANCE inst,LPVOID pData);
	BOOL deviceRead(IDirectInputDevice8 *pDIDevice, void* pBuffer, long lSize) ;
	void dxInputCleanUp();
public:
	HRESULT dxInputInit(HWND hWnd,HINSTANCE hInstance);
	CTRLS dxInputUpdate();
	DXINPUT(HWND hWnd,HINSTANCE hInstance,DWORD modeFlags);
	DXINPUT(HWND hWnd,HINSTANCE hInstance);
	DXINPUT(){}
	~DXINPUT();
};


bool DXINPUT::bGameCtrl;
LPDIRECTINPUT8  DXINPUT::g_pDirectInput;
LPDIRECTINPUTDEVICE8 DXINPUT::g_pJoyStick = NULL;
DIJOYSTATE2  DXINPUT::g_diJoyStickState;
LPDIRECTINPUTDEVICE8 DXINPUT::g_pKeyboardDevice = NULL;  
char DXINPUT::g_pKeyStateBuffer[256] = {0};
HWND DXINPUT::g_hWnd;
HINSTANCE DXINPUT::g_hInstance;
DWORD DXINPUT::dwFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

DXINPUT::DXINPUT(HWND hWnd,HINSTANCE hInstance,DWORD modeFlags)
{
	dwFlags = modeFlags;
	dxInputInit(hWnd,hInstance);
}

DXINPUT::DXINPUT(HWND hWnd,HINSTANCE hInstance)
{
	dxInputInit(hWnd,hInstance);
}

DXINPUT::~DXINPUT()
{
	dxInputCleanUp();
}

HRESULT DXINPUT::dxInputInit(HWND hWnd,HINSTANCE hInstance)
{
	//初始化参数
	bGameCtrl = false;
	g_hWnd = hWnd;
	g_hInstance = hInstance;
	//创建DirectInput接口和设备
	DirectInput8Create(hInstance, 0x0800, IID_IDirectInput8, (void**)&g_pDirectInput, NULL);
	
	//创建键盘设备
    g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, NULL);  
    // 设置数据格式和协作级别  
    g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);  
    g_pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);  
    //获取设备控制权  
    g_pKeyboardDevice->Acquire(); 

	//为游戏控制器初始化
	if(DI_OK != g_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL,
		enumDeviceCallBack,NULL,DIEDFL_ATTACHEDONLY))
	{
		return S_FALSE;
	}

	return S_OK;
}

BOOL DXINPUT::deviceRead(IDirectInputDevice8 *pDIDevice, void* pBuffer, long lSize) 
{
	HRESULT hr;
	while (true) 
	{
		pDIDevice->Poll();              // 轮询设备
		pDIDevice->Acquire();           // 获取设备的控制权
		if (SUCCEEDED(hr = pDIDevice->GetDeviceState(lSize, pBuffer))) break;
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED) return FALSE;
		if (FAILED(pDIDevice->Acquire())) return FALSE;
	}
	return TRUE;
}

CTRLS DXINPUT::dxInputUpdate()
{
	CTRLS ctrls = {-1};
	//bGameCtrl = true;

	ctrls.y = 65535/2;
	ctrls.x = 65535/2;

	if(bGameCtrl)
	{
		::ZeroMemory(&g_diJoyStickState, sizeof(g_diJoyStickState));
		deviceRead(g_pJoyStick, (LPVOID)&g_diJoyStickState, sizeof(g_diJoyStickState));
		
		for(int i = 0;i < 12;i++)
		{
			if(g_diJoyStickState.rgbButtons[i] & 0x80)
			{
				ctrls.keyId = i;
			}
		}

		ctrls.x = g_diJoyStickState.lX;

		ctrls.y = g_diJoyStickState.lY;
	}
	::ZeroMemory(&g_pKeyStateBuffer, sizeof(char) * 256);
	deviceRead(g_pKeyboardDevice, (LPVOID)&g_pKeyStateBuffer, sizeof(g_pKeyStateBuffer));
	

	if(g_pKeyStateBuffer[DIK_UPARROW] & 0x80)
	{
		ctrls.y = 65535/2 - 9362*3;
	}

	if(g_pKeyStateBuffer[DIK_DOWNARROW] & 0x80)
	{
		ctrls.y = 65535/2 + 9362*3;
	}

	if(g_pKeyStateBuffer[DIK_LEFTARROW] & 0x80)
	{
		ctrls.x = 65535/2 - 8000;
	}

	if(g_pKeyStateBuffer[DIK_RIGHTARROW] & 0x80)
	{
		ctrls.x = 65535/2 + 8000;
	}

	return ctrls;
}

void DXINPUT::dxInputCleanUp()
{
	if(bGameCtrl)
	{
		g_pJoyStick->Unacquire();
		g_pKeyboardDevice->Unacquire();
	}
}

BOOL FAR PASCAL DXINPUT::enumDeviceCallBack(LPCDIDEVICEINSTANCE inst,LPVOID pData)
{
	
	//创建设备
	if(DI_OK == g_pDirectInput ->CreateDevice(inst->guidInstance,&g_pJoyStick,NULL))
	{
		//设置数据格式
		if(g_pJoyStick->SetDataFormat(&c_dfDIJoystick2) == DI_OK)
		{
			//设置写作等级
			if(DI_OK == g_pJoyStick->SetCooperativeLevel(g_hWnd, dwFlags))
			{
				//获取设备控制权
				if(DI_OK == g_pJoyStick->Acquire())
				{
					bGameCtrl = true;
					return DIENUM_STOP;
				}
			}
		}
	}

	return DIENUM_CONTINUE;
}