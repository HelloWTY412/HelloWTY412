#pragma once
#include<WinSock2.h>
#include<memory>
using namespace std;

enum class ETYPE{
	ETypeTCP=1,
	ETypeUDP
};


class CSockaddrIn {
public:
	CSockaddrIn() {
		memcpy(&m_addr, 0, sizeof(m_addr));
		m_port = -1;
	};
	CSockaddrIn(sockaddr_in addr) {
		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	CSockaddrIn(UINT nIP, short nPort) {
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);//udp端口最好大一些，一万以上，四万以下
		m_addr.sin_addr.s_addr = htonl(nIP);
		inet_ntoa(m_addr.sin_addr);
		m_port = nPort;
	};
	CSockaddrIn(const string& strIP, short nPort) {
		m_ip = strIP;
		m_port = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);//udp端口最好大一些，一万以上，四万以下
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	};
	CSockaddrIn(const CSockaddrIn& addr) {
		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = addr.m_ip;
		m_port = addr.m_port;
	}
	CSockaddrIn& operator=(const CSockaddrIn& addr) {
		if (this != &addr) {
			memcpy(&m_addr, &addr, sizeof(addr));
			m_ip = addr.m_ip;
			m_port = addr.m_port;
		}
		return *this;
	}

	operator sockaddr* ()const {
		return(sockaddr*)&m_addr;
	};
	operator void* ()const {
		return(void*)&m_addr;
	}
	void update() {
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	string GetIP()const {
		return m_ip;
	};
	short GetPort()const {
		return m_port;
	};
	inline int size()const {
		return sizeof(sockaddr_in);
	}
private:
	sockaddr_in m_addr;
	string  m_ip;
	short m_port;
};

class CBuffer :public string {
public:
	CBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	CBuffer(size_t size=0) :string() {
		if (size > 0) {
			resize(size);
			memset(*this, 0, this->size());
		};
	};
	CBuffer(void* buffer, size_t size): string() {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	~CBuffer() {
		string::~basic_string();
	}
	operator char* () const { return (char*)c_str(); }
	operator const char* () const {return c_str();}
	operator BYTE* () const { return (BYTE*)c_str(); };
	operator void* () const { return (void*)c_str(); };
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};
class CSocket
{
public:
	CSocket(ETYPE nType= ETYPE::ETypeTCP,int nProtocol=0) {
		m_socket = socket(PF_INET, (int)nType, nProtocol);
		m_type = nType;
		m_protocol = nProtocol;
	}
	CSocket(const CSocket& sock) {
		m_socket = socket(PF_INET, (int)m_type, m_protocol);
		m_type = sock.m_type;
		m_protocol = sock.m_protocol;
		m_addr = sock.m_addr;
	}
	~CSocket() {
		close();
	}
	CSocket& operator=(const CSocket& sock) {
		if (this != &sock) {
			m_socket = socket(PF_INET, (int)m_type, m_protocol);
			m_type = sock.m_type;
			m_protocol = sock.m_protocol;
			m_addr = sock.m_addr;
		}
		return *this;
	};
	operator SOCKET() const { return m_socket; };
	operator SOCKET()  { return m_socket; };
	bool operator==(SOCKET sock) const{
		return m_socket == sock;
	}
	int listen(int backlog=5) {
		if (m_type == ETYPE::ETypeTCP) return -1;
		return ::listen(m_socket,backlog);
	}
	int bind(const string& ip, short port) {
		m_addr = CSockaddrIn(ip, port);
		return ::bind(m_socket, m_addr, m_addr.size());
	}
	int accept() {};
	int connect(const string& ip, short port) {};
	int send(const CBuffer& buffer) {
		return ::send(m_socket, buffer, buffer.size(),0);
	};
	int recv(CBuffer& buffer) {
		return ::recv(m_socket, buffer, buffer.size(), 0);
	}
	int sendto(const CBuffer& buffer,const CSockaddrIn& to) {
		return ::sendto(m_socket, buffer, buffer.size(), 0,to,to.size());
	}
	int recvfrom(CBuffer& buffer,  CSockaddrIn& from) {
		int len = from.size();
		int ret= ::recvfrom(m_socket, buffer, buffer.size(), 0, from, &len);
		if (ret > 0) {
			from.update();
		}
		return ret;
	}
	void close() {
		if (m_socket != INVALID_SOCKET) {
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
private:
	SOCKET m_socket;
	ETYPE m_type;
	int m_protocol;
	CSockaddrIn m_addr;
};

typedef shared_ptr<CSocket> CSOCKET;

