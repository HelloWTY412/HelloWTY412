#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include<vector>
#include<map>
#include<list>
#include"Tool.h"
using namespace std;
#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2)//发哦下哦那个报数据应答
#pragma pack(push)
#pragma pack(1)


class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0){};
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
		TRACE("cmd:%d cmd:%d\r\n", nCmd ,sCmd);
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
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
			if (i + 8 > nSize) {//nlength scmd ssum 包数据可能不全，或者包头未能全部收到
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
			sSum = *(WORD*)(pData + i);  i += 2;
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
	CPacket& operator = (const CPacket& pack) {
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
	const char* Data(string& strOut) const{//
		strOut.resize(nLength + 6);
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
	//string strOut;//整个包的数据(自带的缓冲区)
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

typedef struct PacketData {
	string strData;
	UINT nMode;
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nParam = 0)
	{
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;

enum {
	CSM_AUTOCLOSE = 1,//CSM=Client Socket Mode自动关闭模式

};
string GetErrInfo(int wsaErrCode) ;
class CClientSocket
{
public:
	static CClientSocket* getInstance() {//静态方法没有this指针
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	};

	bool InitSocket();
#define BUFFER_SIZE 1048576//1MB
	
	int DealCommand() {
		if (m_sock == -1) return -1;
		char* buffer = m_buffer.data();//多线程发送命令可能会引发冲突
		static size_t index = 0;//buffer中实际
		while (true) {
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);//len :收到的数据大小
			if (((int)len <= 0 )&&((int) index<=0)) return -1;//接受不到数据且缓冲区没数据
			//TODO:处理命令
			index += len;//index: buffer中实际存储的数据大小
			len = index;
			CTool::Dump((BYTE*)buffer, len);
			m_packet = CPacket((BYTE*)buffer, len);//len：输入：buffer中实际存储的数据大小 输出：本次处理的包的大小
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;//index: buffer中剩余的数据的大小
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool GetFilePath(string& strPath) {
		if (m_packet.sCmd > 2 && m_packet.sCmd < 5) {
			strPath = m_packet.strData;
			return TRUE;
		}
		return FALSE;
	};

	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return TRUE;
		}
		return FALSE;
	};
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAdress(DWORD nIP,int nPort) {
		if ((m_nIP!=nIP)||(m_nPort!=nPort)) {
			m_nIP = nIP;
			m_nPort = nPort;
		}
	}
	bool SendPacket(HWND hWnd,const CPacket& pack,bool isAutoClosed=true, WPARAM wParam=0);
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	HANDLE m_eventInvoke;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);//回调函数
	map<UINT, MSGFUNC> m_mapFunc;//消息
	HANDLE m_hThread;//线程
	UINT m_nThreadID;
	bool m_bAutoClose;
	list<CPacket> m_lstSend;
	map<HANDLE, list<CPacket>> m_mapAck;
	DWORD m_nIP;
	int m_nPort;
	vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) ;
	CClientSocket(const CClientSocket& ss);
	CClientSocket();
	~CClientSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		WSACleanup();
	}
	static unsigned __stdcall threadEntry(void* arg);
	//void threadFunc();
	void threadFunc2();
	BOOL InitSockEvn() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		};
		return TRUE;

	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;

		}
	}
	bool Send(const char* pData, int nsize) {
		if (m_sock == -1) return false;
		return send(m_sock, pData, nsize, 0) > 0;
	}

	bool Send(const CPacket& pack) {
		TRACE("m_sock:%d sCmd:%d\r\n", m_sock, pack.sCmd);
		if (m_sock == -1) return false;
		string strOut;
		pack.Data(strOut);
		return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
	}
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};

	static CClientSocket* m_instance;
	static CHelper m_helper;
};

