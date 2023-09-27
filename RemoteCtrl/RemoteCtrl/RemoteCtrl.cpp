﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;
typedef struct file_info {
    file_info(){
        IsInvalid =FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    };
    BOOL IsInvalid;//是否有效
    BOOL IsDirectory;//是否是文件夹 
    BOOL HasNext;//是否有下一个
    char szFileName[256];//文件名
}FILEINFO,*PFILEINFO;

using namespace std;
void Dump(BYTE* pData,size_t nSize) {
    string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if (i>0&&(i%16==0))
        {
            strOut += "\n";
        };
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0XFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}
int MakeDriverInfo() {//1==>A 2==>B 3==>C .... 26==>Z
    string result;
    for (size_t i = 1; i < 26; i++)
    {
        if (_chdrive(i) == 0) {//如果返回0，则存在该磁盘分区
            if (result.size() > 0) result += ',';//分割
            result += 'A' + i - 1;
        };

    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());
   CServerSocket::getInstance()->Send(pack);
    return 0;
}
#include<stdio.h>
#include<io.h>
#include<list>
int MakeDirctoryInfo() {
    string strPath;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误，当前命令不是获取文件列表"));
        return -1;
    };
    if (_chdir(strPath.c_str())!=0) {
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有访问当前文件夹权限"));
        return -2;
    }

    _finddata_t fdata;
    int hfind = 0;
    if ((hfind =_findfirst("*", &fdata))==-1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    };

    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib&_A_SUBDIR)!=0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind,&fdata));
    FILEINFO finfo;
    finfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile() {
    string strPath;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误,打开文件失败"));
        return -1;
    };
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL,SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
};

int DownLoadFile() {
    string strPath;
    long long data = 0;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误,下载文件失败"));
        return -1;
    };
    FILE* pFile = NULL;
    errno_t err=fopen_s(&pFile,strPath.c_str(), "rb");//读，二进制方式打开
    if (err!=0) {
        CPacket pack(4, (BYTE*) &data, 8);
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("打开文件失败"));
        return -1;
    }

    if (pFile != NULL) {
        //先发送文件大小
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(head);
        fseek(pFile, 0, SEEK_SET);

        char buffer[1024];//每次发1k的包
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
    }

    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;

}
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            //socket,bind,listen,accept,read,write,close
            //套接字初始化
            
            //CServerSocket* pserver = CServerSocket::getInstance();
            //int count = 0;
            //if (pserver->InitSocket() == false) {
            //    MessageBox(NULL, _T("网络初始化异常"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}

            //while (CServerSocket::getInstance()!=NULL) {
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("多次无法正常接入"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法正常接入，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealCommand();
            //    //TODO:

            //}
            


        }
        int nCmd = 1;
        switch (nCmd) {
        case 1://查看磁盘分区
            MakeDriverInfo();
            break;
        case 2:
            MakeDirctoryInfo();
            break;
        case 3:
            RunFile();
            break;
        case 4:
            DownLoadFile();
            break;
        }
       

    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
