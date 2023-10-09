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
			//{WM_SEND_PACK,&CClientController::OnSendPack},
			//{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatcher},
			{(UINT) - 1,NULL}
		};
		for (size_t i = 0; MsgFuncs[i].func!=NULL; i++)
		{
			m_mapFunc.insert(pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg, MsgFuncs[i].func));
		}
	}
	return m_instance;
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

//LRESULT CClientController::SendMessage(MSG msg)
//{
//	HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);//�����¼�
//	if (hEvent == NULL) return -2;
//	MSGINFO info(msg);
//	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM) &info, (LPARAM)&hEvent);
//	WaitForSingleObject(hEvent, INFINITE);//�ȴ��¼�����
//	CloseHandle(hEvent);//�����¼��������ֹ��Դ�ľ�
//	return LRESULT();
//}

bool CClientController::SendCommandPacket(HWND hWnd, int nCmd, bool bAutoClose, BYTE* pData ,size_t nLength, WPARAM wParam)//�������ݴ洢��plstPacks��
{
	CClientSocket* pClient = CClientSocket::getInstance();
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	bool ret= pClient->SendPacket(hWnd,CPacket(nCmd, pData, nLength),bAutoClose, wParam);
	return ret;
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, NULL,
		strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_remoteDlg);
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == NULL) {
			AfxMessageBox("������Ȩ�޷��ʻ��ļ��޷�������");
			return -1;
		}
		SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
		//m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadEntryForDownFile, 0, this);
		/*if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT) {
			return -1;
		}*/
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("��������ִ��!"));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();
	}
	return 0;
}
void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadEntryForWatchScreen, 0, this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);//�����߳�
}

void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("ִ�����", _T("���")));

}

//void CClientController::threadDownFile()
//{
//	FILE* pFile = fopen(m_strLocal, "wb+");
//	if (pFile == NULL) {
//		m_statusDlg.ShowWindow(SW_HIDE);
//		m_remoteDlg.EndWaitCursor();
//		AfxMessageBox("������Ȩ�޷��ʻ��ļ��޷�������");
//		return;
//	}
//	CClientSocket* pClient = CClientSocket::getInstance();
//	
//	do {
//		int ret = SendCommandPacket(m_remoteDlg,4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
//		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
//		TRACE("Client nLength=%d\r\n", nLength);
//		if (nLength == 0) {
//			m_statusDlg.ShowWindow(SW_HIDE);
//			m_remoteDlg.EndWaitCursor();
//			AfxMessageBox("�ļ�����Ϊ0���޷���ȡ�ļ���");
//			break;
//		}
//		long long nCount = 0;
//		while (nCount < nLength)
//		{
//			ret = pClient->DealCommand();
//			if (ret < 0) {
//				TRACE("����ʧ�ܣ�ret=%d\r\n", ret);
//				m_statusDlg.ShowWindow(SW_HIDE);
//				m_remoteDlg.EndWaitCursor();
//				AfxMessageBox("����ʧ�ܣ�");
//				break;
//			}
//			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
//			nCount += pClient->GetPacket().strData.size();
//		}
//	}while(false);
//		fclose(pFile);
//		pClient->CloseSocket();
//		m_statusDlg.ShowWindow(SW_HIDE);
//		m_remoteDlg.EndWaitCursor();
//		m_remoteDlg.MessageBox(_T("ִ�����", _T("���")));
//		m_remoteDlg.LoadFileCurrent();
//}

//void CClientController::threadEntryForDownFile(void* arg)
//{
//	CClientController* thiz = (CClientController*)arg;
//	thiz->threadDownFile();
//	_endthread();
//}

void CClientController::threadWatchScreen()//������WatchDlg��m_image��д��
{
	Sleep(50);
	ULONGLONG nTick = GetTickCount64();
	while (!m_isClosed) {
		//if (GetTickCount64() - tick < 50) {//���Ӽ��
		//	Sleep(GetTickCount64() - tick);
		//}
		if (m_watchDlg.isFull() == false) {
			if (GetTickCount64() - nTick < 200) {//���ⷢ��Ƶ�ʹ���(50ms����һ��)
				Sleep(200 -DWORD(GetTickCount64()-nTick));
			}	
			nTick = GetTickCount64();
			int ret=SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			//�����Ϣ��Ӧ����WM_SEND_PACK_ACK
			//���Ʒ���Ƶ��
		if (ret == 1) {	}
			else {
				TRACE(_T("��ȡͼƬʧ��\r\n"));
			}
		}
    Sleep(1);
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
			SetEvent(hEvent);//�¼�����
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

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}
//
//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pBuffer = (char*)wParam;
//	return pClient->Send(pBuffer,(int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//����
	return m_watchDlg.DoModal();
}
