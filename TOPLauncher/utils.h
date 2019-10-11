#pragma once

#include <QString>
#include <string>

#if _MSC_VER < 1920
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#else
#include <filesystem>
namespace filesystem = std::filesystem;
#endif // _MSC_VER < 1920


namespace TOPLauncher
{
    namespace util
    {
        std::string wstringToUTF8(const std::wstring& wstr);
        std::wstring wstringFromUTF8(const std::string& utf8Str);

        QString GetWorkDirectory();
        QString GetAppDirectory();

        QString GetTempDirectory();

        QString GetGameStartupArgs(const QString& serverAddress, const QString& username, const QString& password);

        QString GetSystemLanguageName();

        void GetCurrentExeVersion(int& value1, int& value2, int& value3, int& value4);

    }

}