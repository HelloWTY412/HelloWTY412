#pragma once
#include "pch.h"
#include "framework.h"

using namespace std;
class CPacket {
public:
	CPacket():sHead(0), nLength(0),sCmd(0), sSum(0){};
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {//nSize 输入：pData的字节。输出：用掉多少字节
		size_t i = 0;//i是当前读取到的长度
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
			if (i+8 >= nSize) {//nlength scmd ssum 包数据可能不全，或者包头未能全部收到
				nSize = 0;
				return;
			}

			nLength = *(DWORD*)(pData + i); i += 4;
			if (nLength + i > nSize) {//包未完全收到，返回,解析失败
				nSize = 0;
				return;
			}
			sCmd = *(WORD*)(pData + i); i += 2;
			if (nLength > 4) {
				strData.resize(nLength - 4);//调整字符串大小
				memcpy((void*)strData.c_str(), pData + i, nLength - 4);//将string转换成const char*指针类型
				i += nLength - 4;
			}
			sSum= *(WORD*)(pData + i);  i += 2;
			WORD sum = 0;

			for (size_t j = 0; j < strData.size(); j++)//校验
			{
				sum += BYTE(strData[i]) & 0xFF;
			}

			if (sum = sSum) {
				nSize = i;
				return;
			}
			nSize = 0;

		}
	};
	~CPacket() {};
	CPacket& operator = (const CPacket & pack){
		if (this != &pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		}
		return *this;
	};
	WORD sHead;//包头0xFEFF
	DWORD nLength;//长度（命令开始，校验结束）
	WORD sCmd;//命令
	string strData;//数据
	WORD sSum;//校验
private:


};
class CServerSocket
{
public:
	static CServerSocket* getInstance() {//静态方法没有this指针
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	};

	bool InitSocket() {
		// CServerSocket::getInstance();
		 
		if (m_sock == -1) return false;
		//TODO:校验
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;//监听所有IP（不能假定只有一个IP）
		serv_adr.sin_port = htons(4120);
		//绑定
		if(bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr))==-1)  return false;
		//TODO:
		if(listen(m_sock, 1)==-1) return false;
		return true;
	}

	bool AcceptClient() {
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client =accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1) return false;
		return true;
		
		
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//buffer中实际
		while (true) {
			size_t len=recv(m_client, buffer+index, BUFFER_SIZE -index, 0);//len :收到的数据大小
			if (len <= 0) return -1;
			//TODO:处理命令
			index += len;//index: buffer中实际存储的数据大小
			len = index;
			m_packet=CPacket ((BYTE*)buffer, len);//len：输入：buffer中实际存储的数据大小 输出：本次处理的包的大小
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE -len);
				index -= len;//index: buffer中剩余的数据的大小
				return m_packet.sCmd;
			}
			return -1;
		}

	}

	bool Send(const char* pData, int nsize) {
		return send(m_client, pData, nsize, 0) > 0;
	}
private:
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
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
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
