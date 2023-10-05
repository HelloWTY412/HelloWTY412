#pragma once
#include<Windows.h>
#include<string>
#include<atlimage.h>
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

   static int Bytes2Image(CImage& image, const string& strBuffer) {//数据转图片
	   BYTE* pData = (BYTE*)strBuffer.c_str();
	   HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	   if (hMem == NULL) {
		   TRACE("内存不足");
		   Sleep(1);//防止一直循环消耗CPU资源
		   return -1;
	   }
	   IStream* pStream = NULL;
	   HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	   if (hRet == S_OK) {
		   ULONG length = 0;
		   pStream->Write(pData, strBuffer.size(), &length);
		   TRACE("Client picture size:%d\r\n", strBuffer.size());
		   LARGE_INTEGER bg = { 0 };
		   pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		   if ((HBITMAP)image != NULL) image.Destroy();
		   image.Load(pStream);
	   }
	   return hRet;
   }
};

