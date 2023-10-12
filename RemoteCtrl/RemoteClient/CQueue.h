#pragma once
#include"pch.h"
#include<atomic>

using namespace std;
template<class T>
class CQueue
{//�̰߳�ȫ���У�����IOCPʵ�֣�
	enum {
	EQNone;
	EQPush;
	EQPop;
	EQSize;
	EQClear;
	};
public:
	typedef struct IocpParam {
		size_t nOperator;//����
		T Data;//����
		HANDLE hEvent;//pop������Ҫ
		IocpParam(int op, const T& data,HANDLE hEve=NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//����Ͷ����Ϣ�Ľṹ��
	
public:
	CQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThreaad = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
			m_hThread = (HANDLE)_beginthread(&CQueue<T>::threadEntry,0, m_hCompeletionPort);
		}
	};
	~CQueue() {
		if (m_lock) return;
		m_lock = true;
		HANDLE hTemp = m_hCompeletionPort;
		PostQueuedCompletionStatus(m_hCompeletionPort,0,NULL,NULL);
		WaitForSingleObject(m_hCompeletionPort,INFINITE);
		m_hCompeletionPort = NULL;
		CloseHandle(hTemp);
	};
	bool PushBack(const T& data) {//�����ݣ�Ч��Ҫ�ߣ���ֹ��������������WaitForSingleObject
		if (m_lock)  return false;
		IocpParam* pParam = new IocpParam(EQPush, strData);
		BOOL ret=PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;

	};//��ӽ�����
	bool PopFront(T& data) {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL, NULL);
		IocpParam Param(EQPop, data, hEvent);
		if (m_lock) {
			if (hEvent) CloseHandle(hEvent);
			return false;
		}
		BOOL ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false) { 
			CloseHandle(hEvent);
			return false;
		};
		ret=WaitForSingleObject(hEvent, INFINITE)==WAIT_OBJECT_0;//ȡ����Ч�ʿ��Ա���������һ��
		if (ret) {
			data == Param.Data;
		}
		return ret;
	};//�Ӷ����Ƴ�

	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL, NULL);
		IocpParam Param(EQSize, T(), hEvent);
		if (m_lock) {
			if(hEvent) CloseHandle(hEvent);
			  return -1;
		}
		BOOL ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		};
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//ȡ����Ч�ʿ��Ա���������һ��
		if (ret) {
			return Param.nOperator;
		}
		return -1;

	};//���д�С
	void Clear() {
		if (m_lock)  return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		BOOL ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;
	};//��ն���
	
private:
	static void threadEntry(void arg) {
		CQueue<T>* thiz = (CQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	};//�߳����
	void threadMain() {
	DWORD dwTransferred = 0;
	PParam* pParam = NULL;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOverlapped = NULL;
	while (GetQueuedCompletionStatus(
		m_hCompeletionPort, 
		&dwTransferred, 
		&CompletionKey,
		&pOverlapped,
		INFINITE)) {
		if ((dwTransferred == 0) || (CompletionKey == NULL)) {
			printf("thread is prepare to exit\r\n");
			break;
		}
		pParam = (PPARAM*)CompletionKey;
		switch (pParam->nOperator) {
		case EQPush:
			m_lstData.push_back(pParam->strData);
			delete pParam;
			break;
		case EQPop:
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) {
				setEvent(pParam->hEvent);
			}
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL) {
				setEvent(pParam->hEvent);
			}
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			break;
		default:
			OutPutDebugString("unknown error\r\n");
			break;
		}
	}
	CloseHandle(m_hCompeletionPort);
};//�̹߳���
private:
	list<T> m_lstData;
	HANDLE m_hCompeletionPort;//iocp���
	HANDLE m_hThread;//�߳̾��
	atomic<bool> m_lock;//������������
};

