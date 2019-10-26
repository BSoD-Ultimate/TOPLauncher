#include "stdafx.h"
#include "Environment.h"

namespace TOPLauncher
{
    struct TempDirectory
    {
        TempDirectory(const filesystem::path& d)
            : m_tmpDirPath(d)
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
        filesystem::path m_tmpDirPath;
    };

    static filesystem::path GetOSTempDirectory()
    {
        static const int maxChar = 1000;
        std::unique_ptr<wchar_t[]> tmpPathBuf(new wchar_t[maxChar]());
        std::unique_ptr<wchar_t[]> longTmpPathBuf(new wchar_t[maxChar]());
        GetEnvironmentVariableW(L"temp", tmpPathBuf.get(), maxChar);
        GetLongPathNameW(tmpPathBuf.get(), longTmpPathBuf.get(), maxChar);

        filesystem::path tmpPath = longTmpPathBuf.get();

        return tmpPath;
    }

    /*
     * Environment
    */

    std::shared_ptr<Environment> Environment::GetInstance()
    {
        static std::shared_ptr<Environment> instance(new Environment());
        return instance;
    }
    Environment::Environment()
    {
        auto tempDir = GetOSTempDirectory();
        DWORD pid = GetCurrentProcessId();
        tempDir /= "TOPLauncher_pid" + std::to_string(pid); 
        m_pTempDir = std::make_unique<TempDirectory>(tempDir);
    }
    Environment::~Environment()
    {
    }
    QString Environment::GetWorkingDirectory()
    {
        size_t strLength = GetCurrentDirectoryW(0, NULL);
        std::unique_ptr<wchar_t[]> buf(new wchar_t[strLength]);
        GetCurrentDirectoryW(strLength, buf.get());
        return QString::fromStdWString(std::wstring(buf.get(), strLength - 1));
    }
    QString Environment::GetExecutableDirectory()
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

        filesystem::path parentPath = appPath.parent_path();

        return QString::fromStdWString(std::wstring(parentPath.wstring()));
    }
    QString Environment::GetAppTempDirectory()
    {
        auto tempDir = m_pTempDir->Get();

        QString tmpPath = QString::fromStdWString(tempDir.wstring());
        return tmpPath;
    }

    QString Environment::GetBackgroundPicDirectory()
    {
        filesystem::path appTempPath = GetWorkingDirectory().toStdWString();
        return QString::fromStdWString((appTempPath / "Backgrounds").wstring());
    }
}