#pragma once
#include"pch.h"
#include<atomic>
#include<list>
#include"CThread.h"

using namespace std;

template<class T>
class CQueue
{//线程安全队列（利用IOCP实现）
	
public:
	enum {
	EQNone,
	EQPush,
	EQPop,
	EQSize,
	EQClear
	};
	typedef struct IocpParam {
		size_t nOperator;//操作
		T Data;//数据
		HANDLE hEvent;//pop操作需要
		IocpParam(int op, const T& data,HANDLE hEve=NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//用于投递信息的结构体
public:
	CQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
			m_hThread = (HANDLE)_beginthread(&CQueue<T>::threadEntry,0, this);
		}
	};
	virtual ~CQueue() {
		if (m_lock) return;
		m_lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort,0,NULL,NULL);
		WaitForSingleObject(m_hCompeletionPort,INFINITE);
		if (m_hCompeletionPort != NULL) {
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;
			CloseHandle(hTemp);
		}
	};
	bool PushBack(const T& data) {//拿数据，效率要高（防止阻塞），不适用WaitForSingleObject
		IocpParam* pParam = new IocpParam(EQPush, data);
		if(m_lock) { 
			delete pParam;
			return false;
		};
		bool ret=PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;

	};//添加进队列
	virtual bool PopFront(T& data) {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
		ret=WaitForSingleObject(hEvent, INFINITE)==WAIT_OBJECT_0;//取数据效率可以比拿数据慢一点
		if (ret) {
			data == Param.Data;
		}
		return ret;
	};//从队列移除

	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//取数据效率可以比拿数据慢一点
		if (ret) {
			return Param.nOperator;
		}
		return -1;

	};//队列大小
	bool Clear() {
		if (m_lock)  return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		BOOL ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;
	};//清空队列
	
protected:
	static void threadEntry(void* arg) {
		CQueue<T>* thiz = (CQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	};//线程入口

	virtual void DealParam(PPARAM* pParam) {
		switch (pParam->nOperator) {
		case EQPush:
			m_lstData.push_back(pParam->Data);
			delete pParam;
			break;
		case EQPop:
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			break;
		default:
			OutputDebugStringA("unknown error\r\n");
			break;
		}
	}

	 void threadMain() {
	DWORD dwTransferred = 0;
	PPARAM* pParam = NULL;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOverlapped = NULL;
	while (GetQueuedCompletionStatus(
		m_hCompeletionPort, 
		&dwTransferred, 
		&CompletionKey,
		&pOverlapped,
		INFINITE)) 
	{
		if ((dwTransferred == 0) || (CompletionKey == NULL)) {
			printf("thread is prepare to exit\r\n");
			break;
		}
		pParam = (PPARAM*)CompletionKey;
		DealParam(pParam);
	}
	while (GetQueuedCompletionStatus(
		m_hCompeletionPort,
		&dwTransferred,
		&CompletionKey,
		&pOverlapped, 0)) 
	{//处理残留数据
		if ((dwTransferred == 0) || (CompletionKey == NULL)) {
			printf("thread is prepare to exit\r\n");
			continue;
		}
		pParam = (PPARAM*)CompletionKey;
		DealParam(pParam);
	}
	HANDLE hTemp = m_hCompeletionPort;
	m_hCompeletionPort = NULL;
	CloseHandle(hTemp);
}//线程功能
protected:
	list<T> m_lstData;
	HANDLE m_hCompeletionPort;//iocp句柄
	HANDLE m_hThread;//线程句柄
	atomic<bool> m_lock;//队列正在析构
};



template<class T>
class SendQueue :public CQueue<T>, public ThreadFuncBase {
public:
	typedef int(ThreadFuncBase::* EDYCALLBACK)(T& data);

	SendQueue(ThreadFuncBase* obj, EDYCALLBACK callback)
		:CQueue<T>(), m_base(obj), m_callback(callback) {
	
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE) &SendQueue<T>::threadTick));
	}

	virtual ~SendQueue() {
		m_base = NULL; 
		m_callback = NULL;
		m_thread.Stop();
	};
protected:
	virtual bool PopFront(T& data) { return false; };
	bool PopFront(){
		typename CQueue<T>::IocpParam* Param=new typename CQueue<T>::IocpParam( CQueue<T>::EQPop, T());
		if ( CQueue<T>::m_lock) {
			delete Param;
			return false;
		}
		BOOL ret = PostQueuedCompletionStatus(CQueue<T>::m_hCompeletionPort, sizeof(*Param), (ULONG_PTR)&Param, NULL);
		if (ret == false) {
			delete Param;
			return false;
		};
		return ret;
	}

	int threadTick() {
		if ((WaitForSingleObject(CQueue<T>::m_hThread, 0) != WAIT_TIMEOUT))
			return 0;
		if (CQueue<T>::m_lstData.size() > 0) {
			PopFront();
		}
		//Sleep(1);
		return 0;
	}

	virtual void DealParam(typename CQueue<T>::PPARAM* pParam) {
		switch (pParam->nOperator) {
		case CQueue<T>::EQPush:
			CQueue<T>::m_lstData.push_back(pParam->Data);
			delete pParam;
			break;
		case CQueue<T>::EQPop:
			if (CQueue<T>::m_lstData.size() > 0) {
				pParam->Data = CQueue<T>::m_lstData.front();
				if ((m_base->*m_callback)(pParam->Data)==0) {
					CQueue<T>::m_lstData.pop_front();
				};
			}
			delete pParam;
			break;
		case CQueue<T>::EQSize:
			pParam->nOperator = CQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL) {
				SetEvent(pParam->hEvent);
			}
			break;
		case CQueue<T>::EQClear:
			CQueue<T>::m_lstData.clear();
			delete pParam;
			break;
		default:
			OutputDebugStringA("unknown error\r\n");
			break;
		}
	}
private:
	ThreadFuncBase* m_base;
	EDYCALLBACK m_callback;
	CThread m_thread;
};

typedef SendQueue<vector<char>>::EDYCALLBACK SENDCALLBACK;