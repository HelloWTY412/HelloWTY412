#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include"Packet.h"

using namespace std;


typedef void (*SOCKET_CALLBACK)(void*, int, list<CPacket>&,CPacket&);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {//静态方法没有this指针
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	};

	
int Run(SOCKET_CALLBACK callback, void* arg, short port = 4120) {
		//TODO: 在此处为应用程序的行为编写代码。
		  //socket,bind,listen,accept,read,write,close
		  //套接字初始化
		bool ret = InitSocket(port);
		if (ret == false) return -1;
		list<CPacket> lstPacket;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
	while (true) {
			if (AcceptClient() == false) {//没连接到
				if (count >= 3)
				{
					return -2;//连接3次以上
				}
				count++;
			}	
		int ret = DealCommand();
		if (ret > 0) 
		{
			m_callback(m_arg, ret, lstPacket, m_packet);
			while (lstPacket.size() > 0) 
			{
				Send(lstPacket.front());
				lstPacket.pop_front();
			}
		 }
		 CloseClient();
	}
	return 0;
}
protected:
	bool InitSocket(short port) {
		// CServerSocket::getInstance();

		if (m_sock == -1) return false;
		//TODO:校验
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		//serv_adr.sin_addr.s_addr = INADDR_ANY;//监听所有IP（不能假定只有一个IP）
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
		//绑定
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)  return false;
		//TODO:
		if (listen(m_sock, 1) == -1) return false;
		return true;
	}

	bool AcceptClient() {
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client =accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client=%d\r\n",m_client);
		if (m_client == -1) return false;
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() {//??????????
		
		if (m_client == -1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足！\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//buffer中实际
		while (true) {
			size_t len=recv(m_client, buffer+index, BUFFER_SIZE -index, 0);//len :收到的数据大小
			if (len <= 0) {
				delete[] buffer;
				return -1;
			}
			
			//TODO:处理命令
			index += len;//index: buffer中实际存储的数据大小
			len = index;
			m_packet=CPacket ((BYTE*)buffer, len);//len：输入：buffer中实际存储的数据大小 输出：本次处理的包的大小
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE -len);
				index -= len;//index: buffer中剩余的数据的大小
				delete[] buffer;
				TRACE("m_client:%d recvlen:%d sCmd%d \r\n", m_client, len, m_packet.sCmd);
				return m_packet.sCmd;
			}
		}
		delete[] buffer;
		return -1;
	}

	bool Send(const char* pData, int nsize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nsize, 0) > 0;
	}

	bool Send( CPacket& pack) {
		if (m_client == -1) return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}

	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		};
	}
private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {};
	CServerSocket(const CServerSocket& ss) {	
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEvn() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		};
		 m_sock = socket(PF_INET, SOCK_STREAM, 0);
	};
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEvn() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
			return FALSE;
		};
		return TRUE;
		
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		
		}
	}

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	
	static CServerSocket* m_instance;
	static CHelper m_helper;
};
