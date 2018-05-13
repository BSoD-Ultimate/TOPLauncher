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

        struct TempDirectory
        {
            TempDirectory()
                : m_tmpDirPath(FindTempDirectory())
            {
                if (!filesystem::exists(m_tmpDirPath))
                {
                    filesystem::create_directories(m_tmpDirPath);
                }
            }
            ~TempDirectory()
            {
                filesystem::remove_all(m_tmpDirPath);
            }

            filesystem::path Get() const
            {
                return m_tmpDirPath;
            }

        private:
            filesystem::path FindTempDirectory()
            {
                static const int maxChar = 1000;
                std::unique_ptr<wchar_t[]> tmpPathBuf(new wchar_t[maxChar]());
                std::unique_ptr<wchar_t[]> longTmpPathBuf(new wchar_t[maxChar]());
                GetEnvironmentVariableW(L"temp", tmpPathBuf.get(), maxChar);
                GetLongPathNameW(tmpPathBuf.get(), longTmpPathBuf.get(), maxChar);

                filesystem::path tmpPath = longTmpPathBuf.get();

                DWORD pid = GetCurrentProcessId();

                tmpPath /= wstring_format(L"TOPLauncher_pid{}", pid);

                return tmpPath;
            }
        private:
            filesystem::path m_tmpDirPath;
        };

        std::wstring GetTempDirectory()
        {
            static TempDirectory dir;

            return dir.Get();
        }

        std::wstring GetGameStartupArgs(const std::wstring& serverAddress, const std::wstring& username, const std::wstring& password)
        {
            return wstring_format(L" enc ip={} id={} pw={}", serverAddress, username, password);
        }

        const std::vector<std::pair<std::wstring, std::wstring>>& GetAvailableLanguages()
        {
            static const std::vector<std::pair<std::wstring, std::wstring>> availableLanguages
            {
                { L"en-US", L"English" },
                { L"zh-CN", L"简体中文" },
                { L"zh-TW", L"繁體中文" },
                { L"ja-JP", L"日本語" },
                { L"ko-KR", L"한국어" },
            };

            return availableLanguages;
        }
        std::wstring GetSystemPreferredLanguage()
        {
            LANGID langId = GetUserDefaultUILanguage();
            if (LOBYTE(langId) == LANG_ENGLISH)
            {
                // English
                return GetAvailableLanguages()[0].first;
            }
            else if (langId == 0x0804)
            {
                // simplified chinese
                return GetAvailableLanguages()[1].first;
            }
            else if (langId == 0x0404 || langId == 0x0C04)
            {
                // traditional chinese
                return GetAvailableLanguages()[2].first;
            }
            else if (langId == 0x0404 || langId == 0x0C04)
            {
                // japanese
                return GetAvailableLanguages()[3].first;
            }
            else if (langId == 0x0412)
            {
                // korean
                return GetAvailableLanguages()[4].first;
            }
            else
            {
                return GetAvailableLanguages()[0].first;
            }
        }

        int GetLanguageIndex(const std::wstring & langId)
        {
            auto& availableLanguages = util::GetAvailableLanguages();

            auto iter = std::find_if(availableLanguages.cbegin(), availableLanguages.cend(),
                [&langId](const std::pair<std::wstring, std::wstring>& lang)
            {
                return lang.first == langId;
            });

            if (iter != availableLanguages.cend())
            {
                return iter - availableLanguages.cbegin();
            }
            else
            {
                return -1;
            }
        }

        std::wstring GetLanguageShowString(const std::wstring & langId)
        {
            auto& availableLanguages = util::GetAvailableLanguages();

            auto iter = std::find_if(availableLanguages.cbegin(), availableLanguages.cend(),
                [&langId](const std::pair<std::wstring, std::wstring>& lang)
            {
                return lang.first == langId;
            });

            if (iter != availableLanguages.cend())
            {
                return iter->second;
            }
            else
            {
                return L"Unknown language";
            }
        }

    }

}