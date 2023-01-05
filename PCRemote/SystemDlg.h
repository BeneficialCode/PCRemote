#pragma once
#include"include/IOCPServer.h"
#define AFX_DESIGN_TIME
// CSystemDlg 对话框

class CSystemDlg : public CDialog
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);;   // 标准构造函数
	virtual ~CSystemDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list_process;
private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	int m_iHow;
public:
	void AdjustList();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
private:
	void GetProcessList();
public:
	virtual BOOL OnInitDialog();
	void OnReceiveComplete();
private:
	void ShowProcessList();
public:
	afx_msg void OnKillprocess();
	afx_msg void OnRefreshpslist();
	afx_msg void OnNMRClickListProcess(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	void ShowWindowsList();
public:
	afx_msg void OnWindowClose();
	afx_msg void OnWindowHide();
	afx_msg void OnWindowMax();
	afx_msg void OnWindowMin();
	afx_msg void OnWindowReflush();
	afx_msg void OnWindowReturn();
protected:
	void GetWindowsList();
};
