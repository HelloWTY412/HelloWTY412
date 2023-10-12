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
       if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {//拿token
           ShowError();
           return false;
       };
       TOKEN_ELEVATION eve;
       DWORD len = 0;
       if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len)) {//根据token拿Information
           ShowError();
           return false;
       }
       CloseHandle(hToken);
       if (len == sizeof(eve)) {//判断
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
           MessageBox(NULL, _T("登陆错误"), _T("程序错误"), 0);
           ::exit(0);
       }
       OutputDebugString(L"Logon administartor success!\r\n");*/
       //本地策略组 开启Administrator账户 进制空密码只能登陆本地控制台
      STARTUPINFO si = { 0 };
      PROCESS_INFORMATION pi = { 0 };
      TCHAR sPath[MAX_PATH];
      //GetCurrentDirectory(MAX_PATH, sPath);//获取当前运行进程路径
      GetModuleFileName(NULL, sPath, MAX_PATH);
      //CString strCmd = sPath;
      //strCmd += _T("\\RemoteCtrl.exe");
      //ret=CreateProcessWithTokenW(hToken,LOGON_WITH_PROFILE,NULL,(LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
      BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
      //CloseHandle(hToken);
      if (!ret) {
          ShowError();
          MessageBox(NULL, _T("创建进程失败"), _T("程序错误"), 0);
         return false;
      }
      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return true;
  }

  static void ShowError() {
      LPWSTR lpMessageBuf = NULL;
      //strerror(errno);//标准C语言库
      FormatMessage(//windows显示错误
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
          NULL, GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPWSTR)&lpMessageBuf, 0, NULL);//错误查找
      OutputDebugString(lpMessageBuf);
      LocalFree(lpMessageBuf);
  }

  static bool WriteStartupDir(const CString& strPath) {//修改开机启动文件夹实现开机启动
      TCHAR sPath[MAX_PATH] = _T("");
      GetModuleFileName(NULL, sPath, MAX_PATH);
      return CopyFile(sPath, strPath, FALSE);
  }

 static bool WriteRegisterTable(const CString& strPath) {//修改注册表实现开机启动

      CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
      TCHAR sPath[MAX_PATH] = _T("");
      GetModuleFileName(NULL, sPath, MAX_PATH);
      BOOL ret = CopyFile(sPath, strPath, FALSE);
      if (ret == false) {
          MessageBox(NULL, _T("复制文件失败，是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      HKEY hKey = NULL;
       ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
      if (ret != ERROR_SUCCESS) {
          RegCloseKey(hKey);
          MessageBox(NULL, _T("设置开机自动启动失败\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
      if (ret != ERROR_SUCCESS) {
          RegCloseKey(hKey);
          MessageBox(NULL, _T("设置开机自动启动失败\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
          return false;
      }
      RegCloseKey(hKey);
      return true;
  }

 static bool Init() {//用于带MFC命令行项目的初始化(通用)
     HMODULE hModule = ::GetModuleHandle(nullptr);
     if (hModule == nullptr) {
         wprintf(L"错误: GetModuleHandle 失败\n");
         return false;
     }

     if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
     {
         // TODO: 在此处为应用程序的行为编写代码。
         wprintf(L"错误: MFC 初始化失败\n");
         return false;
     }
     return true;
 }
};

