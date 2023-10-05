#include "pch.h"
#include "ClientController.h"
#include"ClientSocket.h"

map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		struct {
			UINT nMsg; MSGFUNC func;
		}MsgFuncs[] = {
			{WM_SEND_PACK,&CClientController::OnSendPack},
			{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatcher},
			{(UINT) - 1,NULL}
		};
		for (size_t i = 0; MsgFuncs[i].func!=NULL; i++)
		{
			m_mapFunc.insert(pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg, MsgFuncs[i].func));
		}
	}
	return nullptr;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,
		&CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS,&m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);//创建事件
	if (hEvent == NULL) return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM) &info, (LPARAM)&hEvent);
	WaitForSingleObject(hEvent, -1);//等待事件结束
	return LRESULT();
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	CWatchDialog dlg(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(CClientController::threadEntryForWatchScreen, 0, this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);//结束线程
}

void CClientController::threadDownFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile == NULL) {
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		AfxMessageBox("本地无权限访问或文件无法创建！");
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	
	do {
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		TRACE("Client nLength=%d\r\n", nLength);
		if (nLength == 0) {
			m_statusDlg.ShowWindow(SW_HIDE);
			m_remoteDlg.EndWaitCursor();
			AfxMessageBox("文件长度为0或无法读取文件！");
			break;
		}
		long long nCount = 0;
		while (nCount < nLength)
		{
			ret = pClient->DealCommand();
			if (ret < 0) {
				TRACE("传输失败：ret=%d\r\n", ret);
				m_statusDlg.ShowWindow(SW_HIDE);
				m_remoteDlg.EndWaitCursor();
				AfxMessageBox("传输失败！");
				break;
			}
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();
		}
	}while(false);
		fclose(pFile);
		pClient->CloseSocket();
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		m_remoteDlg.MessageBox(_T("执行完毕", _T("完成")));
}

void CClientController::threadEntryForDownFile(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownFile();
	_endthread();
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed) {
		//if (GetTickCount64() - tick < 50) {//增加间隔
		//	Sleep(GetTickCount64() - tick);
		//}
		if (m_remoteDlg.isFull() == false) {
			int ret=SendCommandPacket(6);
			if (ret == 6) {
				if (GetImage(m_remoteDlg.GetImage()) == 0) {
					m_remoteDlg.SetImageStatus(true);
				}
				else {
					TRACE(_T("获取图片失败\r\n"));
				}
			}
			else {
				Sleep(1);
			}
		}
		else Sleep(1);
	}
}

void CClientController::threadEntryForWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE*)msg.lParam;
			map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				pmsg->result=(this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);//事件结束
		}
		else {
			map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
		
	};
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket* pPacket = (CPacket*)wParam;
	return pClient->Send(*pPacket);
}

LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	char* pBuffer = (char*)wParam;
	return pClient->Send(pBuffer,(int)lParam);
}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//阻塞
	return m_watchDlg.DoModal();
}
