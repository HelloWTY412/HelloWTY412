#pragma once
#include "CThread.h"
#include<map>
#include"CQueue.h"
#include<MSWSock.h>
#include<list>



enum Operator{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};

class CServer;
class Client;

typedef shared_ptr<Client> PCLIENT;

class OverLapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;//操作,见Operator
    vector<char> m_buffer;//缓冲区
    ThreadWorker m_worker;//处理函数
    CServer* m_server;//服务器对象
    Client* m_client;//对应的客户端
    WSABUF m_wsabuffer;//
    virtual ~OverLapped() {
        m_buffer.clear();
    }
};

template<Operator> class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<Operator> class RecvOverlapped;
typedef RecvOverlapped <ERecv> RECVOVERLAPPED;

template<Operator> class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;


class Client:public ThreadFuncBase {
public:
    Client();

    ~Client() {
        m_buffer.clear();
        closesocket(m_sock);
        m_recv.reset();
        m_send.reset();
        m_overlapped.reset();   
        m_vecSend.Clear();
    };

    void SetOverlapped(PCLIENT& ptr);

    operator SOCKET() {
        return m_sock;
    }
    operator PVOID() {
        return &m_buffer[0];
    }

    operator LPOVERLAPPED();
    operator LPDWORD() {
        return &m_recevied;
    }
    LPWSABUF RecvWSABuffer();
    LPWSAOVERLAPPED RecvOverlapped();
    LPWSABUF SendWSABuffer();
    LPWSAOVERLAPPED SendOverlapped();
    DWORD& flags() { return m_flags; }

    sockaddr_in* GetLocalAddr() {
        return &m_laddr;
    }
    sockaddr_in* GetRemoteAddr() {
        return &m_raddr;
    }
    size_t GetBufferSize()const { return m_buffer.size(); }
    int Recv();
    int Send(void* buffer,size_t nSize);
    int SendData(vector<char>& data);
private:
    SOCKET m_sock;
    DWORD m_recevied;
    DWORD m_flags;
    shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    shared_ptr<RECVOVERLAPPED>m_recv;
    shared_ptr<SENDOVERLAPPED>m_send;
    vector<char> m_buffer;
    size_t m_used;//已经使用的缓冲区大小
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool m_isbusy;
    SendQueue<vector<char>> m_vecSend;//发送数据队列
};




template<Operator>
class AcceptOverlapped :public OverLapped,ThreadFuncBase
{
public:
    AcceptOverlapped();
    int AcceptWorker();
};



template<Operator>
class RecvOverlapped :public OverLapped, ThreadFuncBase
{
public:
    RecvOverlapped();

    int RecvWorker() {
        int ret = m_client->Recv();
        return ret;
    }

};


template<Operator>
class SendOverlapped :public OverLapped, ThreadFuncBase
{
public:
    SendOverlapped();

    int SendWorker() {
        //1.send不会立即完成
        return -1;
    }
};

template<Operator>
class ErrorOverlapped :public OverLapped, ThreadFuncBase
{
public:
    ErrorOverlapped() :m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker) {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024);
    }
    int ErrorWorker() {
        return -1;
    }
};
typedef ErrorOverlapped<EError> ERROROVERLAPPED;





class CServer :
    public ThreadFuncBase
{
public:
    CServer(const string& ip="0.0.0.0", short port=4120) :m_pool(10) {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_socket = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    };

    ~CServer();
    bool StartService();
    bool NewAccept();
    void BindNewSocket(SOCKET s);
private:
    void CreatSocket();
   
    int threadIocp();
 
private:
    ThreadPool m_pool;
    HANDLE m_hIOCP;//IOCP句柄
    SOCKET m_socket;//服务器套接字
    map<SOCKET, shared_ptr<Client>>  m_client;//多客户端
    sockaddr_in m_addr;
};

