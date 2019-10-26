#pragma once

#include <QString>

namespace TOPLauncher
{
    struct TempDirectory;

    class Environment
    {
    public:
        static std::shared_ptr<Environment> GetInstance();
    private:
        Environment();
    public:
        ~Environment();

        QString GetWorkingDirectory();
        QString GetExecutableDirectory();

        QString GetAppTempDirectory();
        QString GetBackgroundPicDirectory();

    private:
        std::unique_ptr<TempDirectory> m_pTempDir;
    };
}