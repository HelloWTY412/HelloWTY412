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
};

