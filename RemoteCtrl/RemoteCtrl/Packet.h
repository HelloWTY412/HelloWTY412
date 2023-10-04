#pragma once
#include "pch.h"
#include "framework.h"
#include<string.h>
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
				sHead = *(WORD*)(pData + i); i += 2;
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
