#pragma once
#include"ClientSocket.h"
#include"RemoteClientDlg.h"
#include"WatchDialog.h"
#include"StatusDlg.h"
#include<map>
#include"resource.h"
#include "Tool.h"
using namespace std;

#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示状态
#define WM_SHOW_WATCH (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理
class CClientController
{
public:
	static CClientController* getInstance();
	//初始化操作,开线程
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//发送消息
	LRESULT SendMessage(MSG msg);
	//更新网络服务器的地址
	void UpdateAdress(DWORD nIP,int nPort) {
		CClientSocket::getInstance()->UpdateAdress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if(pClient->InitSocket()==false) return false;
		pClient->Send(pack);

	}

//1查看磁盘分区
//2查看文件
//3运行文件
//4下载文件
//5鼠标
//6监视屏幕
//7锁机
//8解锁
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Bytes2Image(image, pClient->GetPacket().strData.c_str());
	}

	int DownFile(CString strPath);

	void StartWatchScreen();
protected:
	void threadDownFile();
	static void threadEntryForDownFile(void* arg);
	void threadWatchScreen();
	static void threadEntryForWatchScreen(void* arg);

	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}
	CClientController() :
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		 m_isClosed=true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	};
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	};
	//在线程中建立消息循环，处理消息
	void threadFunc();
	//进入线程，线程结束后关闭线程
	static unsigned __stdcall threadEntry(void* arg);

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);//nMsg消息，wParam数据，lParam数据大小
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo{
		MSG msg;//发送的消息
		LRESULT result;//收到的结果
		MsgInfo(MSG m) {
			memcpy(&msg, &m, sizeof(MSG));
			result = 0;
		};
		MsgInfo(const MsgInfo& m) {
			memcpy(&msg, &m.msg, sizeof(MSG));
			result = m.result;
		};
		MsgInfo& operator=(const MsgInfo& m) {
			if (this!=&m) {
				memcpy(&msg, &m.msg, sizeof(MSG));
				result = m.result;
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg,WPARAM wParam,LPARAM lParam);
	static map<UINT, MSGFUNC> m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;//线程
	unsigned m_nThreadID;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//监视是否关闭
	CString m_strRemote;//下载文件的远程路径
	CString m_strLocal;//下载文件的本地保存路径
	class CHelper {
	public:
		CHelper() {
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CClientController* m_instance;
	static CHelper m_helper;
};

