#pragma once

#include <QString>
#include <string>
#include <experimental/filesystem>

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

    }

    namespace filesystem = std::experimental::filesystem;

}