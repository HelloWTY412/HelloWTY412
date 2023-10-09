
// RemoteClientDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"
#include"StatusDlg.h"


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
public:
	
	void LoadFileCurrent();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持	
private:
	//static void threadEntryForDownFile(void* arg);
	//void threadDownFile();
	//static void threadEntryForWatchData(void* arg);
	//void threadWatchData();
	void DealCommand(WORD nCmd,const string& strData,LPARAM lParam);
	void InitUIData();
	void LoadFileInfo();
	CString Str2Tree(const string& driver,CTreeCtrl& tree);
    void UpdateFileInfo(const FILEINFO &finfo, HTREEITEM hParent);
    void UpdateDownloadInfo(const string& strData, FILE* pFile);
    CString GetPath(HTREEITEM hTree);
    void DeleteTreeChildrenItem(HTREEITEM hTreeSelected);
// 实现
protected:
	bool m_isClosed;//监视是否关闭
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
	afx_msg void OnBnClickedBtnStartWatch();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
};
