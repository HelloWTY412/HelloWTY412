#pragma once
using namespace std;
class CTool
{
public:
   static void Dump(BYTE* pData, size_t nSize) {
        string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))
            {
                strOut += "\n";
            };
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0XFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }   

  static  bool IsAdmin() {
       HANDLE hToken = NULL;
       if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {//��token
           ShowError();
           return false;
       };
       TOKEN_ELEVATION eve;
       DWORD len = 0;
       if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len)) {//����token��Information
           ShowError();
           return false;
       }
       CloseHandle(hToken);
       if (len == sizeof(eve)) {//�ж�
           return eve.TokenIsElevated;
       }
       printf("length of tokeninformation is%d\r\n", len);
       return false;
   }

  static bool RunAsAdmin() {
      /* HANDLE hToken = NULL;
       BOOL ret=LogonUser(L"Administrator", NULL,NULL,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken);
       if (!ret) {
           ShowError();
           MessageBox(NULL, _T("��½����"), _T("�������"), 0);
           ::exit(0);
       }
       OutputDebugString(L"Logon administartor success!\r\n");*/
       //���ز����� ����Administrator�˻� ���ƿ�����ֻ�ܵ�½���ؿ���̨
      STARTUPINFO si = { 0 };
      PROCESS_INFORMATION pi = { 0 };
      TCHAR sPath[MAX_PATH];
      //GetCurrentDirectory(MAX_PATH, sPath);//��ȡ��ǰ���н���·��
      GetModuleFileName(NULL, sPath, MAX_PATH);
      //CString strCmd = sPath;
      //strCmd += _T("\\RemoteCtrl.exe");
      //ret=CreateProcessWithTokenW(hToken,LOGON_WITH_PROFILE,NULL,(LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
      BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
      //CloseHandle(hToken);
      if (!ret) {
          ShowError();
          MessageBox(NULL, _T("��������ʧ��"), _T("�������"), 0);
         return false;
      }
      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return true;
  }

  static void ShowError() {
      LPWSTR lpMessageBuf = NULL;
      //strerror(errno);//��׼C���Կ�
      FormatMessage(//windows��ʾ����
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
          NULL, GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPWSTR)&lpMessageBuf, 0, NULL);//�������
      OutputDebugString(lpMessageBuf);
      LocalFree(lpMessageBuf);
  }

  static bool WriteStartupDir(const CString& strPath) {//�޸Ŀ��������ļ���ʵ�ֿ�������
      TCHAR sPath[MAX_PATH] = _T("");
      GetModuleFileName(NULL, sPath, MAX_PATH);
      return CopyFile(sPath, strPath, FALSE);
  }

 static bool WriteRegisterTable(const CString& strPath) {//�޸�ע���ʵ�ֿ�������

      CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
      TCHAR sPath[MAX_PATH] = _T("");
      GetModuleFileName(NULL, sPath, MAX_PATH);
      BOOL ret = CopyFile(sPath, strPath, FALSE);
      if (ret == false) {
          MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲���"), _T("����"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      HKEY hKey = NULL;
       ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
      if (ret != ERROR_SUCCESS) {
          RegCloseKey(hKey);
          MessageBox(NULL, _T("���ÿ����Զ�����ʧ��\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
      if (ret != ERROR_SUCCESS) {
          RegCloseKey(hKey);
          MessageBox(NULL, _T("���ÿ����Զ�����ʧ��\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      RegCloseKey(hKey);
      return true;
  }

 static bool Init() {//���ڴ�MFC��������Ŀ�ĳ�ʼ��(ͨ��)
     HMODULE hModule = ::GetModuleHandle(nullptr);
     if (hModule == nullptr) {
         wprintf(L"����: GetModuleHandle ʧ��\n");
         return false;
     }

     if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
     {
         // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
         wprintf(L"����: MFC ��ʼ��ʧ��\n");
         return false;
     }
     return true;
 }
};

