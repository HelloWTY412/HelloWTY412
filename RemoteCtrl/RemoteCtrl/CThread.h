#pragma once

#include"pch.h"
#include<atomic>
#include<vector>
#include<mutex>
#include<Windows.h>
using namespace std;
class ThreadFuncBase{};
typedef int(ThreadFuncBase::* FUNCTYPE)();

class ThreadWorker {//用于将工作交付给线程处理。将线程中执行的功能函数与线程分离
public:
	ThreadWorker() :thiz(NULL), func(NULL) {};
	ThreadWorker(void* obj, FUNCTYPE f) :thiz((ThreadFuncBase*)obj), func(f) {};
	ThreadWorker(const ThreadWorker& worker) {
		thiz = worker.thiz;
		func = worker.func;
	};
	ThreadWorker& operator=(const ThreadWorker& worker) {
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	};

	int operator()(){
		if (IsValid()) {
			return (thiz->*func)();
	    }
		return -1;
	}

	bool IsValid() const{
		return (thiz != NULL) && (func != NULL);
	}
private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};

class CThread
{
public:
	CThread(){
		m_hThread = NULL;
		m_bStatus = false;
	}
	~CThread() {
		Stop();
	}
	bool Start() {//true 成功 false失败
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
		if(!IsValid()){
			m_bStatus = false;
		}
		return m_bStatus;
	}
	bool IsValid() {//返回true表示有效，返回false表示线程异常或中止
		if (m_hThread == NULL || (m_hThread == INVALID_HANDLE_VALUE)) return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;

	}
	bool Stop() {//线程停止
		if (m_bStatus == false) return true;
		m_bStatus = false;
		DWORD ret= WaitForSingleObject(m_hThread,1000);
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread,-1);
		}
		UpdateWorker();
		return ret == WAIT_OBJECT_0;
	}
	void UpdateWorker(const ::ThreadWorker& worker=::ThreadWorker()) {//更新线程工作内容
		if (m_worker.load() != NULL&&(m_worker.load()!=&worker)) {
			::ThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		if (m_worker.load() == &worker) return;
		if (!worker.IsValid()) {
			m_worker.store(NULL);
			return;
		}
		m_worker.store(new ::ThreadWorker(worker));
	}
	
	bool IsIdle() {//判断线程是否空闲 true空闲 false已经分配了工作
		if (m_worker.load() == NULL)return true;
		return !m_worker.load()->IsValid();
	}
private:
	void ThreadWorker(){//线程执行函数
		while (m_bStatus) {
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();
					if (ret != 0) {
						CString str;
						str.Format(_T("thread found warning code %d\r\n,ret"));
						OutputDebugString(str);
					}
					if (ret < 0) {
						::ThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}
			}
			else {
				Sleep(1);
			}

	}
	}
	static void ThreadEntry(void* arg) {//线程入口函数
		CThread* thiz = (CThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread;//线程句柄
	bool m_bStatus;//线程状态 true正常 false线程被关闭或异常
	atomic<::ThreadWorker*> m_worker;
};


class ThreadPool {//线程池
public:
	ThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			m_threads[i] = new CThread();
		}
	}
	ThreadPool() {};
	~ThreadPool() {
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			delete m_threads[i];
			m_threads[i] = NULL;
		}
		m_threads.clear();
	};
	bool Invoke() {
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->Start() == false) {
				ret = false;
				break;
			}
		}
		if (ret == false) {//都启动成功才能成功,失败则全部关闭
			for (size_t i = 0; i < m_threads.size(); i++)
			{
				m_threads[i]->Stop();
			}
		}
	return ret;
	}
	void Stop(){
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			m_threads[i]->Stop();
		}
	}
	int DispatchWorker(const ThreadWorker& worker) {//分配具体工作。返回-1 分配失败，所有线程都在忙。 大于等于0，分配成功，第n个线程在做
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
		}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) {//检测线程有效性
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	mutex m_lock;
	vector<CThread*> m_threads;

};