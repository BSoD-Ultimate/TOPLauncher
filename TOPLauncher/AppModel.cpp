#include "stdafx.h"
#include "AppModel.h"

#include "dbDef.h"

#include "dbConfig.h"
#include "dbServer.h"
#include "dbUser.h"

#include <SQLiteCpp/Database.h>

#include <set>


namespace TOPLauncher
{
    static const std::wstring gameExecutableName = L"tetris.exe";
    // db name 
    static const std::wstring userDBName = L"User.db";
    // config key
    static const std::wstring configKeyLanguage = L"display_language";
    static const std::wstring configKeyGameExecutablePath = L"game_executable_path";


    // reserved server list
    static const std::vector<db::DBServerData> reservedServerList
    {
        { L"TOP Official Server", L"tetrisonline.pl" },
        { L"Chinese CNC Server", L"tetris.3322.org" },
        { L"Another Chinese Server", L"toc.tetriscn.tk" }
    };


    const std::set<std::wstring> availableLanguages{
        L"zh-CN",
        L"zh-TW",
        L"en-US",
        L"ja-JP",
    };

    struct AppConfig
    {
        std::wstring displayLanguage;

        filesystem::path gameExecutablePath;
        filesystem::path gameDirectory;

        // set tetrominos' handling characteristics
        uint16_t moveSensitivity;
        uint16_t moveSpeed;
        uint16_t softDropSpeed;

        // server list
        std::vector<std::shared_ptr<db::DBServerData>> serverList;

        AppConfig()
            : displayLanguage(L"")
            , moveSensitivity(45)
            , moveSpeed(15)
            , softDropSpeed(10)
        {

        }
    };

    AppModel::AppModel()
        : m_pAppConfig(new AppConfig())
    {
    }


    AppModel::~AppModel()
    {
    }

    std::shared_ptr<AppModel> AppModel::GetInstance()
    {
        static std::shared_ptr<AppModel> instance(new AppModel());
        return instance;
    }

    bool AppModel::InitSavedData()
    {
        m_pUserDB = InitUserDB();

        if (!m_pUserDB)
        {
            return false;
        }

        try
        {
            LoadSavedConfigFromDB();
            LoadSavedServers();

            return true;
        }
        catch (const std::runtime_error& e)
        {
            return false;
        }

    }

    bool AppModel::InitGameConfig()
    {
        try
        {
            FindGameExecutablePath();
            LoadSavedConfigFromGame();

            return true;
        }
        catch (const std::runtime_error& e)
        {
            return false;
        }
    }

    std::shared_ptr<SQLite::Database> AppModel::GetUserDB()
    {
        assert(m_pUserDB);
        if (!m_pUserDB)
        {
            throw SQLite::Exception("Attempt to open a invalid database handle");
        }
        return m_pUserDB;
    }

    filesystem::path AppModel::GetGameExecutablePath() const
    {
        return m_pAppConfig->gameExecutablePath;
    }

    void AppModel::SetGameExecutablePath(const std::wstring& exePath)
    {
        m_pAppConfig->gameExecutablePath = exePath;
        m_pAppConfig->gameDirectory = filesystem::path(exePath).parent_path();

        db::WriteDBConfig(*m_pUserDB, configKeyGameExecutablePath, m_pAppConfig->gameExecutablePath);
        LoadSavedConfigFromGame();
    }

    filesystem::path AppModel::GetGameDirectory() const
    {
        return m_pAppConfig->gameDirectory;
    }

    bool AppModel::AddServer(const db::DBServerData& serverData)
    {
        if (IsReservedServer(serverData.serverName))
        {
            return false;
        }

        auto pData = GetServerData(serverData.serverName);

        if (pData)
        {
            return false;
        }

        m_pAppConfig->serverList.push_back(std::make_shared<db::DBServerData>(serverData));
        db::SaveServerData(serverData);

        return true;
    }

    bool AppModel::RemoveServer(const std::wstring& serverName)
    {
        if (IsReservedServer(serverName))
        {
            return false;
        }

        auto iter = std::find_if(m_pAppConfig->serverList.cbegin(), m_pAppConfig->serverList.cend(),
            [serverName](const std::shared_ptr<db::DBServerData>& data)
        {
            assert(data);
            if (!data)
            {
                return false;
            }
            return data->serverName == serverName;
        });

        if (iter != m_pAppConfig->serverList.cend() && db::RemoveServerData(serverName) && db::RemoveAllUsersInServer(serverName))
        {
            iter = m_pAppConfig->serverList.erase(iter);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool AppModel::ModifyServer(const std::wstring & serverName, const db::DBServerData & newServerData)
    {
        auto iter = std::find_if(m_pAppConfig->serverList.begin(), m_pAppConfig->serverList.end(),
            [serverName](const std::shared_ptr<db::DBServerData>& data)
        {
            assert(data);
            if (!data)
            {
                return false;
            }
            return data->serverName == serverName;
        });

        if (iter == m_pAppConfig->serverList.cend())
        {
            return false;
        }
        else
        {
            *iter = std::make_shared<db::DBServerData>(newServerData);
            db::RemoveServerData(serverName);
            db::SaveServerData(newServerData);
            return true;
        }
    }

    std::shared_ptr<db::DBServerData> AppModel::GetServerData(const std::wstring & serverName)
    {
        auto iter = std::find_if(m_pAppConfig->serverList.cbegin(), m_pAppConfig->serverList.cend(),
            [serverName](const std::shared_ptr<db::DBServerData>& data)
        {
            assert(data);
            if (!data)
            {
                return false;
            }
            return data->serverName == serverName;
        });
        if (iter != m_pAppConfig->serverList.cend())
        {
            return *iter;
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<db::DBServerData>>& AppModel::GetServerData() const
    {
        return m_pAppConfig->serverList;
    }

    bool AppModel::IsReservedServer(const std::wstring& serverName) const
    {
        for (const db::DBServerData& serverData : reservedServerList)
        {
            if (serverData.serverName == serverName)
            {
                return true;
            }
        }
        return false;
    }

    bool AppModel::IsGameConfigAvailable() const
    {
        return !m_pAppConfig->gameExecutablePath.empty();
    }

    void AppModel::GetSensitivityValue(int & moveSensitivity, int & moveSpeed, int & dropSpeed)
    {
        moveSensitivity = m_pAppConfig->moveSensitivity;
        moveSpeed = m_pAppConfig->moveSpeed;
        dropSpeed = m_pAppConfig->softDropSpeed;
    }

    void AppModel::SetSensitivityValue(int moveSensitivity, int moveSpeed, int dropSpeed)
    {
        m_pAppConfig->moveSensitivity = moveSensitivity;
        m_pAppConfig->moveSpeed = moveSpeed;
        m_pAppConfig->softDropSpeed = dropSpeed;

        // TODO: apply new settings in game
    }

    std::shared_ptr<SQLite::Database> AppModel::InitUserDB()
    {
        try
        {
            filesystem::path userDBPath = filesystem::path(util::GetWorkDirectory()) / userDBName;
            auto pDB = std::make_shared<SQLite::Database>(userDBPath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            pDB->setBusyTimeout(5000);

            pDB->exec(db::table_public::config::CreateTableSQL());
            pDB->exec(db::table_user::server::CreateTableSQL());
            pDB->exec(db::table_user::user::CreateTableSQL());

            return pDB;
        }
        catch (const SQLite::Exception& e)
        {
            return nullptr;
        }

        return nullptr;
    }

    void AppModel::LoadSavedConfigFromDB()
    {
        assert(m_pUserDB);

        if (!m_pUserDB)
        {
            throw std::runtime_error("User database connection is not properly Opened!");
        }

        // language
        std::wstring langValue = db::ReadDBConfig(*m_pUserDB, configKeyLanguage);
        m_pAppConfig->displayLanguage = langValue;

        // executable path
        std::wstring executablePath = db::ReadDBConfig(*m_pUserDB, configKeyGameExecutablePath);
        SetGameExecutablePath(executablePath);

    }
    void AppModel::LoadSavedServers()
    {
        assert(m_pUserDB);
        if (!m_pUserDB)
        {
            throw std::runtime_error("User database connection is not properly Opened!");
        }

        std::vector<std::shared_ptr<db::DBServerData>> serverList;
        db::LoadAllServers(serverList);

        // reserved server list
        for (const db::DBServerData& serverData : reservedServerList)
        {
            m_pAppConfig->serverList.push_back(std::make_shared<db::DBServerData>(serverData));
        }

        for (const std::shared_ptr<db::DBServerData>& pServerData : serverList)
        {
            assert(pServerData);
            if (pServerData && !IsReservedServer(pServerData->serverName))
            {
                m_pAppConfig->serverList.push_back(pServerData);
            }
        }
    }
    void AppModel::FindGameExecutablePath()
    {
        if (m_pAppConfig->gameExecutablePath.empty())
        {
            filesystem::path workDir = util::GetWorkDirectory();
            filesystem::path gameExePath = workDir / gameExecutableName;

            if (!filesystem::exists(gameExePath))
            {
                //std::string errorMsg = util::string_format("Could not find game executable \"{}\" in current working directory!",
                //    util::wstringToUTF8(gameExecutableName));
                //throw std::runtime_error(errorMsg);
            }
            else
            {
                SetGameExecutablePath(gameExePath);
            }
        }
    }
    void AppModel::LoadSavedConfigFromGame()
    {
        if (m_pAppConfig->gameExecutablePath.empty())
        {
            int i = 1;
            return;
            //throw std::runtime_error("Game executable is not found!");
        }

        int i = 1;

        // TODO: load modifications set on game
    }
}

