#pragma once
#include<map>
#include"ServerSocket.h"
#include<atlimage.h>
#include <direct.h>
#include"Tool.h"
#include<stdio.h>
#include<io.h>
#include<list>
#include "LockInfoDialog.h"
#include "Resource.h"
#include"Packet.h"

using namespace std;
class CCommand
{
public:
	CCommand();
    ~CCommand() {};
	int ExcuteCommand(int nCmd, list<CPacket>& lstPacket,CPacket& inpacket);
    static void RunCommand(void* arg, int status, list<CPacket>& lstPacket, CPacket& inpacket) {
        CCommand* thiz = (CCommand*)arg;
        if (status > 0) {
            int ret = thiz->ExcuteCommand(status, lstPacket, inpacket);
            if (ret != 0) {
                TRACE("执行命令失败 ret：%d status:%d \r\n", ret, status);
            }
        }
        else {
            MessageBox(NULL, _T("无法正常接入，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
        }
    }
protected:
	typedef int(CCommand::* CMDFUNC)(list<CPacket>& lstPacket, CPacket& inpacket);//成员函数指针
	map<int, CMDFUNC> m_mapFunction;//从命令号到指针的映射
    CLockInfoDialog dlg;
    unsigned threadid ;
protected:
   
   static unsigned _stdcall threadLockDlg(void* arg) {
       CCommand* thiz = (CCommand*)arg;
       thiz->threadLockDlgMain();
        _endthreadex(0);
        return 0;
    }

   void threadLockDlgMain() {
       dlg.Create(IDD_DIALOG_INFO, NULL);
       dlg.ShowWindow(SW_SHOW);
       //遮蔽后台
       CRect rect;
       rect.left = 0;
       rect.top = 0;
       rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//w1
       rect.bottom = GetSystemMetrics(SM_CXFULLSCREEN);
       rect.bottom = LONG(rect.bottom * 1.10);
       dlg.MoveWindow(rect);
       CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
       if (pText) {
           CRect rtText;
           pText->GetWindowRect(rtText);
           int nWidth = rtText.Width();//w0
           int nHeight = rtText.Height();
           int x = (rect.right - nWidth) / 2;
           int y = (rect.bottom - nHeight) / 2;
           pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
       };
       //窗口置顶
       dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
       //限制鼠标
       ShowCursor(false);
       //隐藏任务栏
       ::ShowWindow(::FindWindow(_T("shell_traywnd"), NULL), SW_HIDE);
       //限制鼠标活动范围
       dlg.GetWindowRect(rect);
       rect.left = 0;
       rect.top = 0;
       rect.right = 1;
       rect.bottom = 1;
       ClipCursor(rect);
       MSG msg;
       while (GetMessage(&msg, NULL, 0, 0)) {
           TranslateMessage(&msg);
           DispatchMessage(&msg);
           if (msg.message == WM_KEYDOWN) {
               TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
               if (msg.wParam == 0x41) {//按a退出
                   break;
               }
           }

       }
       ClipCursor(NULL);
       //恢复鼠标
       ShowCursor(TRUE);
       //回复任务栏
       ::ShowWindow(::FindWindow(_T("shell_traywnd"), NULL), SW_SHOW);
       dlg.DestroyWindow();
   }

    int MakeDriverInfo(list<CPacket>& lstPacket, CPacket& inpacket) {//1==>A 2==>B 3==>C .... 26==>Z
        string result;
        for (size_t i = 1; i < 26; i++)
        {
            if (_chdrive(i) == 0) {//如果返回0，则存在该磁盘分区
                if (result.size() > 0) result += ',';//分割
                result += 'A' + i - 1;
            };
        }
        result += ',';
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }

    int MakeDirctoryInfo(list<CPacket>& lstPacket, CPacket& inpacket) {
        string strPath=inpacket.strData;
        if (strPath.empty()) {
            OutputDebugString(_T("命令解析错误，当前命令不是获取文件列表"));
            return -1;
        };
        if (_chdir(strPath.c_str()) != 0) {//转到该目录下
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            /* finfo.IsInvalid = TRUE;
             finfo.IsDirectory = TRUE;
             memcpy(finfo.szFileName, strPath.c_str(), strPath.size());*/
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            OutputDebugString(_T("没有访问当前文件夹权限"));
            return -2;
        };
        _finddata_t fdata;
        intptr_t hfind = 0;
        //strPath += "*";
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("没有找到任何文件"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            return -3;
        };
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));           
            TRACE(" send dir [%s]\r\n", finfo.szFileName);
        } while (!_findnext(hfind, &fdata));
        _findclose(hfind); // 关闭文件查找句柄
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
        return 0;
    }

    int RunFile(list<CPacket>& lstPacket, CPacket& inpacket) {
        string strPath=inpacket.strData;
        if (strPath.empty()) {
            OutputDebugString(_T("命令解析错误,打开文件失败"));
            return -1;
        };
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        lstPacket.push_back(CPacket(3, NULL, 0));
        return 0;
    };

    int DownLoadFile(list<CPacket>& lstPacket, CPacket& inpacket) {
        string strPath = inpacket.strData;
        long long data = 0;
        if (strPath.empty()) {
            OutputDebugString(_T("命令解析错误,下载文件失败"));
            return -1;
        };
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");//读，二进制方式打开
        if (err != 0) {
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            OutputDebugString(_T("打开文件失败"));
            return -1;
        }

        if (pFile != NULL) {
            //先发送文件大小
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);
            TRACE("Server nLength=%d\r\n", data);
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            fseek(pFile, 0, SEEK_SET);

            char buffer[1024];//每次发1k的包
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                 lstPacket.push_back(CPacket(4, (BYTE*)buffer, rlen));
            } while (rlen >= 1024);
            fclose(pFile);
        }
        lstPacket.push_back(CPacket(4, NULL, 0));
        return 0;

    }
    int  MouseEvent(list<CPacket>& lstPacket, CPacket& inpacket) {
        MOUSEEV mouse;
        memcpy(&mouse, inpacket.strData.c_str(), sizeof(MOUSEEV));
       
            DWORD nFlages = 0;
            switch (mouse.nButton) {
            case 0://左键
                nFlages = 1;
                break;
            case 1://右键
                nFlages = 2;
                break;
            case 2://中键
                nFlages = 4;
                break;
            case 4://没有案件
                nFlages = 8;
                break;
            }
            if (nFlages != 8) SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
            switch (mouse.nAction) {
            case 0://单击
                nFlages |= 0x10;
                break;
            case 1://双击
                nFlages |= 0x20;
                break;
            case 2://按下
                nFlages |= 0x40;
                break;
            case 3://松开
                nFlages |= 0x80;
                break;
            defult:
                break;
            }
            switch (nFlages) {
            case 0x21://左键双击
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x11://左键单机
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x41://左键按下
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x81://左键松开
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x22://右键双击
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x12://右键单机
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x42://右键按下
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x82://右键松开
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x24://中键双击
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x14://中键单机
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x44://中键按下
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x84://中键松开
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x08://鼠标移动
                mouse_event(MOUSEEVENTF_MOVE, 0, 0, 0, GetMessageExtraInfo());
                break;
            }
            lstPacket.push_back(CPacket(5, NULL, 0));
            return 0;
    }

    int  SendScreen(list<CPacket>& lstPacket, CPacket& inpacket) {
        CImage screen;
        HDC hscreen = ::GetDC(NULL);//拿到当前屏幕句柄
        int nBitPerPixsl = GetDeviceCaps(hscreen, BITSPIXEL);
        int nWidth = GetDeviceCaps(hscreen, HORZRES);
        int nHeight = GetDeviceCaps(hscreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixsl);//创建图像 
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hscreen, 0, 0, SRCCOPY);//复制
        ReleaseDC(NULL, hscreen);//释放hscreen
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == NULL) return-1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//PNG格式
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);
            PBYTE pData = (PBYTE)GlobalLock(hMem);//会返回一个指向被分配内存地址的指针
            SIZE_T nSize = GlobalSize(hMem);
            lstPacket.push_back(CPacket(6, pData, nSize));          
            TRACE("Server picture size:%d\r\n", nSize);
            GlobalUnlock(hMem);
        }
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        return 0;

    }

  
    int LockMachine(list<CPacket>& lstPacket, CPacket& inpacket) {
        if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
            _beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
        lstPacket.push_back(CPacket(7, NULL, 0));
        return 0;
    };
    int UnlockMachine(list<CPacket>& lstPacket, CPacket& inpacket) {
        PostThreadMessage(threadid, WM_KEYDOWN, 0X41, 0);
        lstPacket.push_back(CPacket(8, NULL, 0));
        return 0;
    };

    int  TextConnect(list<CPacket>& lstPacket, CPacket& inpacket) {
        lstPacket.push_back(CPacket((WORD)1981, NULL, 0));
        return 0;
    }
    int DeleteLocalFile(list<CPacket>& lstPacket, CPacket& inpacket) {
        string strPath= inpacket.strData;
        if (strPath.empty()) {
            OutputDebugString(_T("命令解析错误,下载文件失败"));
            return -1;
        };
        //TCHAR sPath[MAX_PATH] = _T("");
        //MultiByteToWideChar(CP_ACP,0,strPath.c_str(),strPath.size(),sPath,sizeof(sPath)/sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }
};

