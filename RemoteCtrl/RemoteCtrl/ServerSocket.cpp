#include "pch.h"
#include "framework.h"
#include "ServerSocket.h"

//CServerSocket server;

CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;

//erverSocket* pserver = CServerSocket::getInstance();