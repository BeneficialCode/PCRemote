// WTSOpenServer.cpp: 定义控制台应用程序的入口点。
//
#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <Wtsapi32.h>

#pragma  comment (lib,"Wtsapi32.lib")


int main()
{
	//WCHAR* szServerName = L"";   //win10 不需要
	WCHAR * szServerName = NULL;
	HANDLE WtsServerHandle = WTSOpenServer(szServerName);

	// 然后开始遍历终端服务器上的所有进程，这里我们是指本机的所有进程.

	PWTS_PROCESS_INFO pWtspi;
	DWORD dwCount;

	if (!WTSEnumerateProcesses(WtsServerHandle, 0, 1, &pWtspi, &dwCount))
	{
		int a = GetLastError();
		return 0;
	}


	for (DWORD i = 0; i < dwCount; i++)
	{
		printf("ProcessID: %d %ls\n", pWtspi[i].ProcessId,
			pWtspi[i].pProcessName);
	}

	getchar();
}

