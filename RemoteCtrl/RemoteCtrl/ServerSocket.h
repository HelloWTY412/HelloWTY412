#pragma once
#include "pch.h"
#include "framework.h"

using namespace std;
#pragma pack(push)
#pragma pack(1)

class CPacket {
public:
	CPacket():sHead(0), nLength(0),sCmd(0), sSum(0){};
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//打包
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)//校验
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
		
	}
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
				sHead = *(WORD*)(pData + i); i += 2;
				break;
			}
		}

			if (i+8 > nSize) {//nlength scmd ssum 包数据可能不全，或者包头未能全部收到
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
				sum += BYTE(strData[j]) & 0xFF;
			}

			if (sum == sSum) {
				nSize = i;
				return;
			}
			nSize = 0;

		
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

	int Size() {
		return nLength + 6;
	};
	const char* Data() {//
		strOut.resize(nLength+6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}

	
	WORD sHead;//包头0xFEFF
	DWORD nLength;//长度（命令开始，校验结束）
	WORD sCmd;//命令
	string strData;//数据
	WORD sSum;//校验
	string strOut;//整个包的数据(自带的缓冲区)
};
#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//鼠标的动作
	WORD nButton;//鼠标的案件
	POINT ptXY;//坐标

}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	};
	BOOL IsInvalid;//是否有效
	BOOL IsDirectory;//是否是文件夹 
	BOOL HasNext;//是否有下一个
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;

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

	bool GetFilePath(string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <=4)||(m_packet.sCmd ==9)) {
			strPath = m_packet.strData;
			return TRUE;
		}
		return FALSE;
	};

	bool GetMouseEvent(MOUSEEV &mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return TRUE;
		}
		return FALSE;
	};
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseClient() {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
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
