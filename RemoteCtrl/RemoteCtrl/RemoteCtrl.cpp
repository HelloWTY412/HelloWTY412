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


int main()
{
    if (!CTool::Init()) return 1;

    

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