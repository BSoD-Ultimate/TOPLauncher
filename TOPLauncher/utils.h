#pragma once

#include <string>
#include <experimental/filesystem>

#include <fmt/format.h>

namespace TOPLauncher
{
    namespace util
    {
        std::string wstringToUTF8(const std::wstring& wstr);
        std::wstring wstringFromUTF8(const std::string& utf8Str);

        template <typename... Args>
        std::string string_format(const char* strFormat, Args... arg)
        {
            std::string formatStr = fmt::format(strFormat, arg...);
            return formatStr;
        }

        template <typename... Args>
        std::wstring wstring_format(const wchar_t* strFormat, Args... arg)
        {
            std::wstring formatStr = fmt::format(strFormat, arg...);
            return formatStr;
        }

        std::wstring GetWorkDirectory();
        std::wstring GetAppDirectory();

        std::wstring GetGameStartupArgs(const std::wstring& serverAddress, const std::wstring& username, const std::wstring& password);
    }

    namespace filesystem = std::experimental::filesystem;

}