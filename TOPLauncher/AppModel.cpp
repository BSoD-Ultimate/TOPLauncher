#include "stdafx.h"
#include "AppModel.h"

#include "dbDef.h"

#include "dbConfig.h"
#include "dbServer.h"
#include "dbUser.h"

#include <SQLiteCpp/Database.h>



namespace TOPLauncher
{
    static const std::wstring gameExecutableName = L"tetris.exe";
    // db name 
    static const std::wstring userDBName = L"User.db";
    // config key
    static const std::wstring configKeyLanguage = L"display_language";


    // reserved server list
    static const std::vector<db::DBServerData> reservedServerList 
    {
        { L"TOP Official Server", L"tetrisonline.pl" },
        { L"Chinese CNC Server", L"tetris.3322.org" },
        { L"Another Chinese Server", L"toc.tetriscn.tk" }
    };

    enum class LanguageConfig
    {
        SimplifiedChinese,
        TraditionalChinese,
        English,
        Japanese,
    };

    struct AppConfig
    {
        LanguageConfig displayLanguage;

        filesystem::path gameExecutablePath;
        filesystem::path gameDirectory;

        // set tetrominos' handling characteristics
        uint16_t moveSensitivity;
        uint16_t moveSpeed;
        uint16_t softDropSpeed;

        // server list
        std::unordered_map<std::wstring, std::shared_ptr<db::DBServerData>> serverMap;

        AppConfig()
            : displayLanguage(LanguageConfig::SimplifiedChinese)
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

        auto iter = m_pAppConfig->serverMap.find(serverData.serverName);

        if (iter != m_pAppConfig->serverMap.cend())
        {
            return false;
        }

        m_pAppConfig->serverMap[serverData.serverName] = std::make_shared<db::DBServerData>(serverData);
        db::SaveServerData(serverData);

        return true;
    }

    bool AppModel::RemoveServer(const std::wstring& serverName)
    {
        if (IsReservedServer(serverName))
        {
            return false;
        }

        return db::RemoveServerData(serverName) && m_pAppConfig->serverMap.erase(serverName) > 0;
    }

    std::shared_ptr<db::DBServerData> AppModel::GetServerData(const std::wstring & serverName)
    {
        auto iter = m_pAppConfig->serverMap.find(serverName);
        if (iter != m_pAppConfig->serverMap.cend())
        {
            return iter->second;
        }
        return nullptr;
    }

    const std::unordered_map<std::wstring, std::shared_ptr<db::DBServerData>>& AppModel::GetServerData() const
    {
        return m_pAppConfig->serverMap;
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
        m_pAppConfig->displayLanguage = LanguageConfig(_wtoi(langValue.c_str()));



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
            m_pAppConfig->serverMap[serverData.serverName] = std::make_shared<db::DBServerData>(serverData);
        }

        for (const std::shared_ptr<db::DBServerData>& pServerData : serverList)
        {
            assert(pServerData);
            if (pServerData && !IsReservedServer(pServerData->serverName))
            {
                m_pAppConfig->serverMap[pServerData->serverName] = pServerData;
            }
        }
    }
    void AppModel::FindGameExecutablePath()
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

