#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

//erverSocket* pserver = CServerSocket::getInstance();

string GetErrInfo(int wsaErrCode) {
	string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

//bool CClientSocket::SendPacket(const CPacket& pack, list<CPacket>& lstPacks)
//{
//	if (m_sock == INVALID_SOCKET) {
//		//if (InitSocket() == false) return false;
//		_beginthread(&CClientSocket::threadEntry, 0, this);
//	}
//	auto pr = m_mapAck.insert(pair<HANDLE, list<CPacket>>(pack.hEvent, list<CPacket>()));
//	m_lstSend.push_back(pack);
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	map<HANDLE, list<CPacket>>::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end()) {
//		list<CPacket>::iterator i;
//		for (i = it->second.begin(); i != it->second.end(); i++)
//		{
//			lstPacks.push_back(*i);
//		}
//		m_mapAck.erase(it);
//		return true;
//	}
//	return false;
//}

CClientSocket& CClientSocket::operator=(const CClientSocket& ss)
{
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++)
	{
		m_mapFunc.insert(pair<UINT, MSGFUNC>(it->first, it->second));
	}
	return *this;
}

CClientSocket::CClientSocket(const CClientSocket& ss)
{
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it!=ss.m_mapFunc.end(); it++)
	{
		m_mapFunc.insert(pair<UINT, MSGFUNC>(it->first, it->second));
	}
};

CClientSocket::CClientSocket() :
m_nIP(INADDR_ANY),
m_nPort(0), m_sock(INVALID_SOCKET) {
	if (InitSockEvn() == FALSE) {
		MessageBox(NULL, _T("无法初始化套接字环境,检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	};
	m_eventInvoke = CreateEvent(NULL, TRUE,FALSE,NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) {
		TRACE("网络消息处理线程启动失败\r\n");
	};
	CloseHandle(m_eventInvoke);
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);

	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClientSocket::SendPack},
		{0,NULL}
	};
	for (size_t i = 0; funcs[i].message!=0; i++)
	{
		if (m_mapFunc.insert(pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("插入失败");
		}
	}
};

bool CClientSocket::InitSocket()
{
	// CServerSocket::getInstance();
	if (m_sock != INVALID_SOCKET)  CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) return false;
	//TODO:校验
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(m_nIP);//监听所有IP（不能假定只有一个IP）
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("指定的IP地址,不存在");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1) {
		AfxMessageBox("连接失败");
		return FALSE;
	}
	return true;
};

bool CClientSocket::SendPacket(HWND hWnd,const CPacket& pack, bool isAutoClosed, WPARAM wParam)
{
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
	string strOut;
	pack.Data(strOut);
	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	bool ret= PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)pData, (LPARAM)hWnd);
	if (ret == false) {
		delete pData;
	}
	return ret;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PACKET_DATA data = *(PACKET_DATA*) wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	size_t nTemp = data.strData.size();
	CPacket current ( (BYTE*)data.strData.c_str(),nTemp);
	if (InitSocket() == true) {
		int ret = send(m_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET) {
				int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
				if (length > 0 || index > 0) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						TRACE("接收到的命令：%d 包的大小：%d hWnd:%08X\r\n",pack.sCmd, nLen,hWnd);
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLOSE) {
							CloseSocket();
							return;
						}
						index -= nLen;
						memmove(pBuffer, pBuffer + nLen, index);
					}
				}
				else {//对方关闭套接字或网络设备异常
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(current.sCmd,NULL,0), 1);
				}
			}
		}
		else {//网络终止处理
			CloseSocket();
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {//错误处理
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
	}
}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}

void CClientSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

//void CClientSocket::threadFunc()
//{
//	string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pBuffer = (char*)strBuffer.c_str();
//	int index = 0;
//	InitSocket();
//	while (m_sock != INVALID_SOCKET) {
//		if (m_lstSend.size() > 0) {
//			CPacket& head = m_lstSend.front();
//			if (Send(head) == false) {
//				TRACE(_T("发送失败 "));
//				continue;
//			}
//			map<HANDLE, list<CPacket>>::iterator it;
//			it = m_mapAck.find(head.hEvent);
//			int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
//			if (length > 0 || index > 0) {
//				index += length;
//				size_t size = (size_t)index;
//				CPacket pack((BYTE*)pBuffer, size);
//				if (size > 0) {//文件夹信息获取可能有问题
//					pack.hEvent = head.hEvent;
//					it->second.push_back(pack);
//					SetEvent(head.hEvent);
//					memmove(pBuffer, pBuffer + size, index - size);
//					index -= size;
//				}
//			}
//			else if (length <= 0 && index <= 0) {
//				CloseSocket();
//			}
//			m_lstSend.pop_front();
//			InitSocket();
//		}
//	}
//	CloseSocket();
//}
