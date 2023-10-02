
// RemoteClientDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"
#include"StatusDlg.h"

#define WM_SEND_PACKET (WM_USER+1)
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	int SendCommandPacket(int nCmd, bool Close=TRUE, BYTE* pData = NULL, size_t nLength = 0);
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTreeSelected);
	void LoadFileInfo();
	void LoadFileCurrent();
private:
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	static void threadEntryForWatchData(void* arg);
	void threadWatchData();
public:
	bool isFull()const {
		return m_isFull;
	}
	CImage& GetImage() {
		return m_image;
	}
	void SetImageStatus(bool isFull=false) {
		m_isFull = isFull;
	}
private:
	CImage m_image;
	bool m_isFull;//false没满，true满了
	bool m_isClosed;//监视是否关闭
// 实现
protected:
	
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnText();
	DWORD m_server_address;
	CString m_nPort;
	CTreeCtrl m_Tree;
	afx_msg void OnBnClickedButtonFileinfo();
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnSelchangedTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam,LPARAM lParam);
	afx_msg void OnBnClickedBtnStartWatch();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
