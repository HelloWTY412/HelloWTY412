#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include<vector>
using namespace std;

#pragma pack(push)
#pragma pack(1)


class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {};
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//���
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
		for (size_t j = 0; j < strData.size(); j++)//У��
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
	CPacket(const BYTE* pData, size_t& nSize) {//nSize ���룺pData���ֽڡ�������õ������ֽ�
		size_t i = 0;//i�ǵ�ǰ��ȡ���ĳ���
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
			if (i + 8 > nSize) {//nlength scmd ssum �����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ���յ�
				nSize = 0;
				return;
			}

			nLength = *(DWORD*)(pData + i); i += 4;
			if (nLength + i > nSize) {//��δ��ȫ�յ�������,����ʧ��
				nSize = 0;
				return;
			}
			sCmd = *(WORD*)(pData + i); i += 2;
			if (nLength > 4) {
				strData.resize(nLength - 4);//�����ַ�����С
				memcpy((void*)strData.c_str(), pData + i, nLength - 4);//��stringת����const char*ָ������
				i += nLength - 4;
			}
			sSum = *(WORD*)(pData + i);  i += 2;
			WORD sum = 0;

			for (size_t j = 0; j < strData.size(); j++)//У��
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
	const char* Data() {//
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}


	WORD sHead;//��ͷ0xFEFF
	DWORD nLength;//���ȣ����ʼ��У�������
	WORD sCmd;//����
	string strData;//����
	WORD sSum;//У��
	string strOut;//������������(�Դ��Ļ�����)
};
#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//���Ķ���
	WORD nButton;//���İ���
	POINT ptXY;//����

}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	};
	BOOL IsInvalid;//�Ƿ���Ч
	BOOL IsDirectory;//�Ƿ����ļ��� 
	BOOL HasNext;//�Ƿ�����һ��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;

string GetErrInfo(int wsaErrCode) ;
class CClientSocket
{
public:
	static CClientSocket* getInstance() {//��̬����û��thisָ��
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	};

	bool InitSocket(int nIP,int nPort) {
		// CServerSocket::getInstance();
		if (m_sock != INVALID_SOCKET)  CloseSocket();
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1) return false;
		//TODO:У��
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(nIP);//��������IP�����ܼٶ�ֻ��һ��IP��
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox("ָ����IP��ַ,������");
			return false;
		}
		int ret = connect(m_sock,(sockaddr*)&serv_adr,sizeof(serv_adr));
		if (ret == -1) {
			AfxMessageBox("����ʧ��");
			return FALSE;
		}
		return true;
	};
	void Dump(BYTE* pData, size_t nSize) {
		string strOut;
		for (size_t i = 0; i < nSize; i++)
		{
			char buf[8] = "";
			if (i > 0 && (i % 16 == 0))
			{
				strOut += "\n";
			};
			snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0XFF);
			strOut += buf;
		}
		strOut += "\n";
		OutputDebugStringA(strOut.c_str());
	}

#define BUFFER_SIZE 819200//100KB
	
	int DealCommand() {//?????????? 
		if (m_sock == -1) return -1;
		char* buffer = m_buffer.data();
		static size_t index = 0;//buffer��ʵ��
		while (true) {
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);//len :�յ������ݴ�С
			if (len <= 0 && index<=0) return -1;//���ܲ��������һ�����û����
			//TODO:��������
			index += len;//index: buffer��ʵ�ʴ洢�����ݴ�С
			len = index;
			//Dump((BYTE*)buffer, len);
			m_packet = CPacket((BYTE*)buffer, len);//len�����룺buffer��ʵ�ʴ洢�����ݴ�С ��������δ���İ��Ĵ�С
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;//index: buffer��ʣ������ݵĴ�С
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, int nsize) {
		if (m_sock == -1) return false;
		return send(m_sock, pData, nsize, 0) > 0;
	}

	bool Send(CPacket& pack) {
		TRACE("m_sock:%d sCmd:%d\r\n", m_sock,pack.sCmd);
		if (m_sock == -1) return false;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
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
private:
	vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) {};
	CClientSocket(const CClientSocket& ss) {
		m_sock = ss.m_sock;
	}
	CClientSocket() {
		if (InitSockEvn() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,�����������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		};
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(),0,BUFFER_SIZE);
	};
	~CClientSocket() {
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
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;

		}
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

