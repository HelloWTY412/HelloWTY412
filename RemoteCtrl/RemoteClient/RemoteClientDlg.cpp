
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include"WatchDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEXT, &CRemoteClientDlg::OnBnClickedBtnText)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemoteClientDlg::OnBnClickedButtonFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DIR, &CRemoteClientDlg::OnTvnSelchangedTreeDir)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_MESSAGE(WM_SEND_PACKET,&CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
//	ON_WM_TIMER()
ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
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

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7F000001;//127.0.0.1
	m_nPort = _T("4120");
	UpdateData(false);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
	m_isFull = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRemoteClientDlg::OnPaint()
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
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//1查看磁盘分区
//2查看文件
//3运行文件
//4下载文件
//5鼠标
//6监视屏幕
//7锁机
//8解锁
int CRemoteClientDlg::SendCommandPacket(int nCmd, bool Close, BYTE* pData,size_t nLength) {
	UpdateData();//true：把控件的值赋给成员变量  false：把成员变量的值赋给控件
	m_server_address;
	atoi((LPCTSTR)m_nPort);
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret) {
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	pClient->Send(pack);
	int cmd = pClient->DealCommand();
	TRACE("cmd:%d ack:%d\r\n", cmd, pClient->GetPacket().sCmd);
	if(Close==TRUE) {
		pClient->CloseSocket();
	}
	return cmd;
}

void CRemoteClientDlg::OnBnClickedBtnText()
{
	// TODO: 在此添加控件通知处理程序代码
	SendCommandPacket(1981);
}


void CRemoteClientDlg::OnBnClickedButtonFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret=SendCommandPacket(1);//获取磁盘信息
	if (ret == -1) {
		AfxMessageBox("命令处理失败");
		return;
	};
	CClientSocket *pClient=CClientSocket::getInstance();
	string drives=pClient->GetPacket().strData;
	string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drives.size(); i++)
	{
		if (drives[i] == ',') {
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);
			m_Tree.InsertItem(NULL, hTemp, TVI_ROOT);
			dr.clear();
			continue;
		}
		dr += drives[i];
	}

}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + "\\" + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree!=NULL);
	return strRet;
}
void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTreeSelected) {
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTreeSelected);
		if (hSub != NULL) {
			m_Tree.DeleteItem(hSub);
			// 在删除后重新获取 hTreeSelected 的子项。根据MFC版本添加该代码
			//hSub = m_Tree.GetChildItem(hTreeSelected);
		}
	} while (hSub != NULL);//注意：GetChildItem函数会返回给定节点的第一个子项，删除节点后，即使不重新获取，hsub也会指向新的子项
}
void CRemoteClientDlg::LoadFileInfo() {
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	if (m_Tree.GetSafeHwnd() == NULL) {
		// 处理错误情况
		AfxMessageBox("树控件错误");
		return;
	}//确保m_tree不为空
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL) return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL) return;
	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems(); 
	//m_Tree.UpdateWindow(); // 强制刷新界面
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFILEINFO pinfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	while (pinfo->HasNext) {
		TRACE("[%s] is dir %d\r\n", pinfo->szFileName, pinfo->IsDirectory);
		if (pinfo->IsDirectory) {//是文件夹
			if (CString(pinfo->szFileName) == '.' || CString(pinfo->szFileName) == "..") {//是.或..的则跳过
				int cmd = pClient->DealCommand();
				if (cmd < 0) break;
				pinfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pinfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
			//m_Tree.UpdateWindow(); // 强制刷新界面
		}
		else {
			m_List.InsertItem(0,pinfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		if (cmd < 0) break;
		pinfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	}

	pClient->CloseSocket();
}

void CRemoteClientDlg::LoadFileCurrent() {
	
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();
	//m_Tree.UpdateWindow(); // 强制刷新界面
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFILEINFO pinfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	while (pinfo->HasNext) {
		TRACE("[%s] is dir %d\r\n", pinfo->szFileName, pinfo->IsDirectory);
		if (!pinfo->IsDirectory) {//是文件夹
			m_List.InsertItem(0, pinfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		if (cmd < 0) break;
		pinfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();
}


void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0) return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup=menu.GetSubMenu(0);
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,ptMouse.x,ptMouse.y,this);

	}
}


void CRemoteClientDlg::OnTvnSelchangedTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();
}


void CRemoteClientDlg::threadDownFile() 
{
	
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	CFileDialog dlg(FALSE, NULL,
		strFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, this);
	if (dlg.DoModal() == IDOK) {
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL) {
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			AfxMessageBox("本地无权限访问或文件无法创建！");
			return;
		}
		HTREEITEM hSelected = m_Tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		TRACE("%s\r\n", strFile);
		CClientSocket* pClient = CClientSocket::getInstance();
		do {
			int ret =SendMessage(WM_SEND_PACKET,4<<1|0,(LPARAM)(LPCSTR)strFile);
			if (ret < 0) {
				TRACE("执行下载命令失败 ret=%d\r\n", ret);
				m_dlgStatus.ShowWindow(SW_HIDE);
				EndWaitCursor();
				AfxMessageBox("执行下载命令失败");
				break;
			};
			
			long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
			TRACE("Client nLength=%d\r\n", nLength);
			if (nLength == 0) {
				m_dlgStatus.ShowWindow(SW_HIDE);
				EndWaitCursor();
				AfxMessageBox("文件长度为0或无法读取文件！");
				break;
			}
			long long nCount = 0;
			while (nCount < nLength)
			{
				ret=pClient->DealCommand();
				if (ret < 0) {
					TRACE("传输失败：ret=%d\r\n", ret);
					m_dlgStatus.ShowWindow(SW_HIDE);
					EndWaitCursor();
					AfxMessageBox("传输失败！");
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
		} while (false);
		fclose(pFile);
		pClient->CloseSocket();
	}
	m_dlgStatus.ShowWindow(SW_HIDE);
	EndWaitCursor();
	MessageBox(_T("执行完毕",_T("完成")));
	
}

void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

void CRemoteClientDlg::threadWatchData() {
	//可能存在异步，导致程序崩溃
	Sleep(50);
	CClientSocket* pClient = NULL;
	do {
		pClient= CClientSocket::getInstance();
	} while (pClient == NULL);
	//ULONGLONG tick = GetTickCount64();
	while(!m_isClosed) {
		//if (GetTickCount64() - tick < 50) {//增加间隔
		//	Sleep(GetTickCount64() - tick);
		//}
		if (m_isFull == false) {
			int  cmd = SendMessage(WM_SEND_PACKET, 6 << 1 | 1);
			if (cmd == 6) {
				BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
				if (hMem == NULL) {
					TRACE("内存不足");
					Sleep(1);//防止一直循环消耗CPU资源
					continue;
				}
				IStream* pStream = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
				if (hRet == S_OK) {
					ULONG length = 0;
					pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
					TRACE("Client picture size:%d\r\n", pClient->GetPacket().strData.size());
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg, STREAM_SEEK_SET, NULL);
					if((HBITMAP)m_image!=NULL) m_image.Destroy();
					m_image.Load(pStream);
					m_isFull = true;
				}

			}
			else {
				Sleep(1);
			}
		}
		else Sleep(1);
	}

}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam) {
	int ret = 0;
	int cmd = wParam >> 1;
	switch (cmd) {
	case 4:
	{
		CString strFile = (LPCSTR)lParam;
		 ret = SendCommandPacket(cmd,wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	}
	break;
	case 5: {
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEV));
	}
		  break;
	case 6:
	{
		 ret = SendCommandPacket(cmd, wParam & 1);
	}
	break;
	default:
		ret = 1;
	}

	return ret;
}

void CRemoteClientDlg::OnDownloadFile()
{
	// TODO: 在此添加命令处理程序代码
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	BeginWaitCursor();
	m_dlgStatus.m_info.SetWindowText(_T("命令正在执行!"));
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();
}


void CRemoteClientDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) AfxMessageBox("删除文件命令失败");
	LoadFileCurrent();
}


void CRemoteClientDlg::OnRunFile()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;
	int ret = SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) AfxMessageBox("打开文件命令失败！");
	
}


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	m_isClosed = false;
	CWatchDialog dlg(this);
	HANDLE hThread=(HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(hThread,500);//结束线程
}





void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
