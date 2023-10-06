#pragma once
#include"ClientSocket.h"
#include"RemoteClientDlg.h"
#include"WatchDialog.h"
#include"StatusDlg.h"
#include<map>
#include"resource.h"
#include "Tool.h"
using namespace std;

#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)//չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����
class CClientController
{
public:
	static CClientController* getInstance();
	//��ʼ������,���߳�
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//������Ϣ
	LRESULT SendMessage(MSG msg);
	//��������������ĵ�ַ
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

//1�鿴���̷���
//2�鿴�ļ�
//3�����ļ�
//4�����ļ�
//5���
//6������Ļ
//7����
//8����
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
	//���߳��н�����Ϣѭ����������Ϣ
	void threadFunc();
	//�����̣߳��߳̽�����ر��߳�
	static unsigned __stdcall threadEntry(void* arg);

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);//nMsg��Ϣ��wParam���ݣ�lParam���ݴ�С
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo{
		MSG msg;//���͵���Ϣ
		LRESULT result;//�յ��Ľ��
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
	HANDLE m_hThread;//�߳�
	unsigned m_nThreadID;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����Ƿ�ر�
	CString m_strRemote;//�����ļ���Զ��·��
	CString m_strLocal;//�����ļ��ı��ر���·��
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

