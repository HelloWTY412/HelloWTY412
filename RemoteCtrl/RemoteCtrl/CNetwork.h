#pragma once
#include"CSocket.h"
#include"CThread.h"
class CNetwork
{

};

typedef int (*AcceptFunc)(void* arg, CSOCKET& client);
typedef int (*RecvFunc)(void* arg, const CBuffer& buffer);
typedef int (*SendFunc)(void* arg, CSOCKET& clients,int ret);
typedef int (*RecvFromFunc)(void* arg, const CBuffer& buffer,CSockaddrIn& addr);
typedef int (*SendToFunc)(void* arg, CSockaddrIn& addr,int ret);

class CServerParamter 
{
public:
	CServerParamter(const string& ip="0.0.0.0",
		short port=4120, 
		ETYPE type=ETYPE::ETypeTCP,
		AcceptFunc acceptf = NULL,
		RecvFunc recvf = NULL,
		SendFunc sendf=NULL,
		RecvFromFunc recvfromf=NULL,
		SendToFunc sendtof=NULL);
	//输入
	CServerParamter& operator<<(AcceptFunc func);
	CServerParamter& operator<<(RecvFunc func);
	CServerParamter& operator<<(SendFunc func);
	CServerParamter& operator<<(RecvFromFunc func);
	CServerParamter& operator<<(SendToFunc func);
	CServerParamter& operator<<(string &ip);
	CServerParamter& operator<<(short port);
	CServerParamter& operator<<(ETYPE type);
	//输出
	CServerParamter& operator>>(AcceptFunc& func);
	CServerParamter& operator>>(RecvFunc& func);
	CServerParamter& operator>>(SendFunc& func);
	CServerParamter& operator>>(RecvFromFunc& func);
	CServerParamter& operator>>(SendToFunc& func);
	CServerParamter& operator>>(string& ip);
	CServerParamter& operator>>(short& port);
	CServerParamter& operator>>(ETYPE& type);
	//复制构造，=重载
	CServerParamter(const CServerParamter& param);
	CServerParamter& operator==(const CServerParamter& param);

	string m_ip;
	short m_port;
	ETYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};


class EServer:public ThreadFuncBase {

public:
	EServer(const CServerParamter& param);
	~EServer();
	int Invoke(void* arg);
	int Send(CSOCKET& client, const CBuffer& buffer);
	int Sendto( CSockaddrIn& addr,const CBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int ThreadUDPFunc();
	int ThreadTCPFunc();
private:
	CServerParamter m_params;
	void* m_args;
	CThread m_thread;
	CSOCKET m_sock;
	atomic<bool> m_stop;
};

