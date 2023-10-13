#include "pch.h"
#include "CNetwork.h"

EServer::EServer(const CServerParamter& param):m_stop(false), m_args(NULL)
{
	m_params = param;
	m_thread.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&EServer::threadFunc));

}

EServer::~EServer()
{
	Stop();
}

int EServer::Invoke(void* arg)
{
    m_sock.reset(new CSocket(m_params.m_type));
    // SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (*m_sock == INVALID_SOCKET) {
        printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    };
    if (m_params.m_type == ETYPE::ETypeTCP) {
        if (m_sock->listen() == -1) {
            return -2;
        };
    }
    CSockaddrIn client;
    if (-1 == m_sock->bind(m_params.m_ip,m_params.m_port)) {
        printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return -3;
    };
    if (m_thread.Start() == false) return -4;
    m_args = arg;
	return 0;
}

int EServer::Send(CSOCKET& client, const CBuffer& buffer)
{
    int ret=m_sock->send(buffer);//���Ż������ͳɹ����ǲ����������
    if (m_params.m_send) m_params.m_send(m_args,client,ret);
    return ret;
}

int EServer::Sendto(CSockaddrIn& addr, const CBuffer& buffer)
{
   int ret= m_sock->sendto(buffer,addr);//���Ż������ͳɹ����ǲ����������
    if (m_params.m_sendto) m_params.m_sendto(m_args, addr, ret);
    return ret;
}

int EServer::Stop()
{
    if (m_stop == false) {
        m_sock->close();
        m_stop = true;
        m_thread, Stop();
    }
    return 0;
}

int EServer::threadFunc()
{
    if (m_params.m_type == ETYPE::ETypeTCP) {
        return ThreadTCPFunc();
    }
    else {
        return ThreadUDPFunc();
    }
  
    return 0;
}

int EServer::ThreadUDPFunc()
{
    CBuffer buf(1024 * 256);
    CSockaddrIn client;
    int ret = 0;
    while (!m_stop) {
        ret = m_sock->recvfrom(buf, client);
        if (ret > 0) {
            client.update();
            if (m_params.m_recvfrom != NULL) {
                m_params.m_recvfrom(m_args,buf,client);
            }
        }
        else {
            printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
            break;
        }

        Sleep(1);
    }
    if (m_stop == false) m_stop = true;
    m_sock->close();
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
}

int EServer::ThreadTCPFunc()
{
    return 0;
}

CServerParamter::CServerParamter(const string& ip, short port,
    ETYPE type, AcceptFunc acceptf, RecvFunc recvf, 
    SendFunc sendf, RecvFromFunc recvfromf, SendToFunc sendtof)
{
    m_ip = ip;
    m_port = port;
    m_type = type;
    m_accept = acceptf;
    m_recv = recvf;
    m_send = sendf;
    m_recvfrom = recvfromf;
    m_sendto = sendtof;

}

CServerParamter& CServerParamter::operator<<(AcceptFunc func)
{
    // TODO: �ڴ˴����� return ���
    m_accept = func;
    return *this;
}

CServerParamter& CServerParamter::operator<<(RecvFunc func)
{
    // TODO: �ڴ˴����� return ���
    m_recv = func;
    return *this;
}

CServerParamter& CServerParamter::operator<<(SendFunc func)
{
    // TODO: �ڴ˴����� return ���
    m_send = func;
    return *this;
}

CServerParamter& CServerParamter::operator<<(RecvFromFunc func)
{
    // TODO: �ڴ˴����� return ���
    m_recvfrom = func;
    return *this;
}

CServerParamter& CServerParamter::operator<<(SendToFunc func)
{
    // TODO: �ڴ˴����� return ���
    m_sendto = func;
    return *this;
}

CServerParamter& CServerParamter::operator<<(string& ip)
{
    // TODO: �ڴ˴����� return ���
    m_ip = ip;
    return *this;
}

CServerParamter& CServerParamter::operator<<(short port)
{
    // TODO: �ڴ˴����� return ���
    m_port = port;
    return *this;
}

CServerParamter& CServerParamter::operator<<(ETYPE type)
{
    // TODO: �ڴ˴����� return ���
    m_type = type;
    return *this;
}

CServerParamter& CServerParamter::operator>>(AcceptFunc& func)
{
    // TODO: �ڴ˴����� return ���
    func = m_accept;
    return *this;
}

CServerParamter& CServerParamter::operator>>(RecvFunc& func)
{
    // TODO: �ڴ˴����� return ���
    func = m_recv;
    return *this;
}

CServerParamter& CServerParamter::operator>>(SendFunc& func)
{
    // TODO: �ڴ˴����� return ���
    func = m_send;
    return *this;
}

CServerParamter& CServerParamter::operator>>(RecvFromFunc& func)
{
    // TODO: �ڴ˴����� return ���
    func = m_recvfrom;
    return *this;
}

CServerParamter& CServerParamter::operator>>(SendToFunc& func)
{
    // TODO: �ڴ˴����� return ���
    func = m_sendto;
    return *this;
}

CServerParamter& CServerParamter::operator>>(string& ip)
{
    // TODO: �ڴ˴����� return ���
    ip = m_ip;
    return *this;
}

CServerParamter& CServerParamter::operator>>(short& port)
{
    // TODO: �ڴ˴����� return ���
    port = m_port;
    return *this;
}

CServerParamter& CServerParamter::operator>>(ETYPE& type)
{
    // TODO: �ڴ˴����� return ���
    type = m_type;
    return *this;
}

CServerParamter::CServerParamter(const CServerParamter& param)
{
    m_ip = param.m_ip;
    m_port = param.m_port;
    m_type = param.m_type;
    m_accept = param.m_accept;
    m_recv = param.m_recv;
    m_send = param.m_send;
    m_recvfrom = param.m_recvfrom;
    m_sendto = param.m_sendto;
}

CServerParamter& CServerParamter::operator==(const CServerParamter& param)
{
    // TODO: �ڴ˴����� return ���
    if (this != &param) {
        m_ip = param.m_ip;
        m_port = param.m_port;
        m_type = param.m_type;
        m_accept = param.m_accept;
        m_recv = param.m_recv;
        m_send = param.m_send;
        m_recvfrom = param.m_recvfrom;
        m_sendto = param.m_sendto;
    }
    return *this;
}
