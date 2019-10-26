#include "stdafx.h"
#include "utils.h"

namespace TOPLauncher
{
    namespace util
    {
        std::string wstringToUTF8(const std::wstring & wstr)
        {
            int stringLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length() + 1, NULL, 0, NULL, NULL);
            std::unique_ptr<char[]> stringBuf(new char[stringLen]());
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), stringBuf.get(), stringLen, NULL, NULL);
            return std::string(stringBuf.get());
        }
        std::wstring wstringFromUTF8(const std::string & utf8Str)
        {
            int stringLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Str.length() + 1, NULL, 0);
            std::unique_ptr<wchar_t[]> stringBuf(new wchar_t[stringLen]());
            MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Str.length(), stringBuf.get(), stringLen);
            return std::wstring(stringBuf.get());
        }

        QString GetGameStartupArgs(const QString& serverAddress, const QString& username, const QString& password)
        {
            return QString(" enc ip=%1 id=%2 pw=%3").arg(serverAddress, username, password);
        }

        QString GetSystemLanguageName()
        {
            int nchars = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, NULL, 0);
            auto languageNameBuf = std::make_unique<wchar_t[]>(nchars);
			GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, languageNameBuf.get(), nchars);

            std::wstring languageName(languageNameBuf.get(), nchars - 1);

            return QString::fromStdWString(languageName);
        }

        void GetCurrentExeVersion(__out int& value1, __out int& value2, __out int& value3, __out int& value4)
        {
            value1 = value2 = value3 = value4 = 0;

            size_t bufSize = MAX_PATH;
            std::unique_ptr<wchar_t[]> exeFilePathBuf(new wchar_t[bufSize]());
            int error = 0;
            do
            {
                GetModuleFileNameW(NULL, exeFilePathBuf.get(), bufSize);
                int error = GetLastError();
                if (error == ERROR_INSUFFICIENT_BUFFER)
                {
                    bufSize += MAX_PATH;
                    exeFilePathBuf.reset(new wchar_t[bufSize]());
                }
            } while (error == ERROR_INSUFFICIENT_BUFFER);

            DWORD fileVersionBufSize = GetFileVersionInfoSizeW(exeFilePathBuf.get(), NULL);
            std::unique_ptr<unsigned char[]> versionBuf(new unsigned char[fileVersionBufSize]());
            GetFileVersionInfoW(exeFilePathBuf.get(), NULL, fileVersionBufSize, versionBuf.get());
            VS_FIXEDFILEINFO* pFileInfo = NULL;
            UINT fileInfoLen = 0;
            BOOL bRet = VerQueryValueW(versionBuf.get(), L"\\", reinterpret_cast<LPVOID*>(&pFileInfo), &fileInfoLen);
            std::unique_ptr<unsigned short[]> verInfo(new unsigned short[4]());
            verInfo[0] = HIWORD(pFileInfo->dwFileVersionMS);
            verInfo[1] = LOWORD(pFileInfo->dwFileVersionMS);
            verInfo[2] = HIWORD(pFileInfo->dwFileVersionLS);
            verInfo[3] = LOWORD(pFileInfo->dwFileVersionLS);

            value1 = verInfo[0];
            value2 = verInfo[1];
            value3 = verInfo[2];
            value4 = verInfo[3];
        }

    }

}