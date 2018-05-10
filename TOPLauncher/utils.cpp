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

        std::wstring GetWorkDirectory()
        {
            size_t strLength = GetCurrentDirectoryW(0, NULL);
            std::unique_ptr<wchar_t[]> buf(new wchar_t[strLength]);
            GetCurrentDirectoryW(strLength, buf.get());
            return std::wstring(buf.get(), strLength - 1);
        }

        std::wstring GetAppDirectory()
        {
            size_t bufLength = 150;
            std::unique_ptr<wchar_t[]> buf(new wchar_t[bufLength]);

            int error = 0;
            do
            {
                GetModuleFileNameW(NULL, buf.get(), bufLength);
                error = GetLastError();
                if (error == ERROR_INSUFFICIENT_BUFFER)
                {
                    bufLength = bufLength * 3 / 2;
                    buf.reset(new wchar_t[bufLength]);
                }
            } while (error == ERROR_INSUFFICIENT_BUFFER);

            filesystem::path appPath = buf.get();

            return appPath.parent_path();
        }

        std::wstring GetGameStartupArgs(const std::wstring & serverAddress, const std::wstring & username, const std::wstring & password)
        {
            return wstring_format(L" enc ip={} id={} pw={}", serverAddress, username, password);
        }

    }

}