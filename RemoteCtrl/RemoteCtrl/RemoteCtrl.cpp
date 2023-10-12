// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include"Command.h"
#include<conio.h>
#include"CQueue.h"
#include"CServer.h"
#include<list>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
// 唯一的应用程序对象
 //#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")
#define INVOKE_PATH  _T("C:\\Users\\vboxuser\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")
CWinApp theApp;

bool ChooseAutoInvoke(const CString& strPath) {
    TCHAR wcsSystem[MAX_PATH] = _T("");
    if (PathFileExists(strPath)) {
        return true;
    }
    CString strInfo = _T("该程序只用于合法用途\n");
    strInfo += ("继续运行该程序，此机器将处于被监控状态\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        //WriteRegisterTable(strPath);
        if (!CTool::WriteStartupDir(strPath)) {
            MessageBox(NULL, _T("复制文件失败，是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        };
    }
    else if (ret == IDCANCEL) {
        return false;
    }
    return true;
}

//void text() {
//    CQueue<string> lstStrings;
//    ULONGLONG tick0 = GetTickCount64(), tick = GetTickCount64();
//    while (_kbhit() == 0) {
//        if (GetTickCount64() - tick0 > 1300) {
//            lstStrings.PushBack("hello world");
//            tick0 = GetTickCount64();
//        }
//        if (GetTickCount64() - tick > 2000) {
//            string str;
//            lstStrings.PopFront(str);
//            tick = GetTickCount64();
//            printf("pop from queue: %s\r\n", str.c_str());
//        }
//        Sleep(1);
//    }
//    printf("exit done size: %d\r\n", lstStrings.Size());
//    lstStrings.Clear();
//    printf("exit done size: %d\r\n", lstStrings.Size());
//}

void udp_server();
void udp_client(bool ishost=true);

void initsock() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}
void clearsock() {
    WSACleanup();
}
int main(int argc,char* argv[])
{
    if (!CTool::Init()) return 1;
    initsock();
    if (argc == 1) {
        char wstrDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, wstrDir);
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        string strCmd = argv[0];
        strCmd += " 1";
        BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
        if (bRet) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TRACE("进程ID：%d\r\n", pi.dwProcessId);
            TRACE("线程ID：%d\r\n", pi.dwThreadId);
            strCmd += " 2";
            BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
            if (bRet) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                TRACE("进程ID：%d\r\n", pi.dwProcessId);
                TRACE("线程ID：%d\r\n", pi.dwThreadId);
                udp_server();//服务器代码
            }
        };
    }
    else if (argc == 2) {//主客户端
        udp_client();
    }
    else {//从客户端
        udp_client(false);
    }

    clearsock();
    /*
    if (CTool::IsAdmin()) {
        if (!CTool::Init()) return 1;
       // OutputDebugString(L"current is run as administartor!\r\n");
        if (ChooseAutoInvoke(INVOKE_PATH)) {
            CCommand cmd;
            int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
            switch (ret) {
            case -1:
                MessageBox(NULL, _T("网络初始化异常"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                break;
            case -2:
                MessageBox(NULL, _T("多次无法正常接入"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                break;
            }
        }
    }
    else {
       // OutputDebugString(L"current is run as normal user!\r\n");
        if (CTool::RunAsAdmin() == false) {
            CTool::ShowError();
            return 1;
        }
    }
    */
    return 0;
}
 

void iocp() {
    CServer server;
    server.StartService();
    getchar();
}

void udp_server() {
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    };
    list<sockaddr_in> lstclients;
    sockaddr_in server, client;
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));
    server.sin_family = AF_INET;
    server.sin_port = htons(20000);//udp端口最好大一些，一万以上，四万以下
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (-1 == bind(sock, (sockaddr*)&server, sizeof(server))) {
        printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        closesocket(sock);
        return;
    };
    string buf;
    buf.resize(1024 * 256);
    memset((char*)buf.c_str(), 0, buf.size());
    int len = sizeof(client);
    int ret = 0;
    while (!_kbhit()) {
        ret = recvfrom(sock,(char*)buf.c_str(), buf.size(), 0, (sockaddr*)&client, &len);
        if (ret > 0) {
            if (lstclients.size() <= 0) {
                lstclients.push_back(client);
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                ret = sendto(sock, buf.c_str(), ret, 0, (sockaddr*)&client, len);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            }
            else {
                memcpy((void*)buf.c_str(), &lstclients.front(), sizeof(lstclients.front()));
                ret = sendto(sock, buf.c_str(), sizeof(lstclients.front()), 0, (sockaddr*)&client, len);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            }
            //CTool::Dump((BYTE*)buf.c_str(), ret);
        }
        else {
            printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        }
       
        Sleep(1);
    }
    closesocket(sock);
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    getchar();
};
void udp_client(bool ishost ) {
  
    Sleep(2000);
    sockaddr_in server,client;
    int len = sizeof(client);
    server.sin_family = AF_INET;
    server.sin_port = htons(20000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("%s(%d):%s ERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }
 
    if (ishost) {//主客户端
     
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        string msg = "hello world\r\n";
        int ret=sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(),0, msg.size());
            ret=recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%s ERROR(%d)!!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            if (ret > 0) {
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg=%s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
            }
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%s ERROR(%d)!!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__,WSAGetLastError(),ret);
            printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            if (ret > 0) {
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg=%s\r\n", __FILE__, __LINE__, __FUNCTION__,msg.c_str());
            }
        }
    }
    else {//从客户端
;        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        string msg = "hello world\r\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("%s(%d):%s msg=%s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
            if (ret > 0) {
                sockaddr_in addr;
                memcpy(&addr, msg.c_str(), sizeof(addr));
                sockaddr_in* paddr = (sockaddr_in*)&addr;
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, client.sin_port);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
                msg = "hello, i am client\r\n";
                ret=sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)paddr, sizeof(sockaddr_in));
                printf("%s(%d):%s ip:%08X port:%d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr,ntohs(paddr->sin_port));
                printf("%s(%d):%s ret:%d\r\n", __FILE__, __LINE__, __FUNCTION__,ret);
            }
        } 
    }
    closesocket(sock);
};