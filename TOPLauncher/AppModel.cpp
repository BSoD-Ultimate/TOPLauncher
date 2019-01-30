#include "stdafx.h"
#include "AppModel.h"

#include "dbDef.h"

#include "dbConfig.h"
#include "dbServer.h"
#include "dbUser.h"

#include "guiUtil.h"
#include "gameConfigUtil.h"

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
        { L"TOP Official Server", L"tetrisonline.pl", L"http://tetrisonline.pl/top/register.php" }
    };




    struct AppConfig
    {
        std::wstring displayLanguage;

        filesystem::path gameExecutablePath;
        filesystem::path gameDirectory;

        // set tetrominos' handling characteristics
        int32_t moveSensitivity;
        int32_t moveSpeed;
        int32_t softDropSpeed;

        // set line-clear delay time
        int32_t lineClearDelay;

        // server list
        std::vector<std::shared_ptr<db::DBServerData>> serverList;

        AppConfig()
            : moveSensitivity(45)
            , moveSpeed(15)
            , softDropSpeed(10)
            , lineClearDelay(0)
        {
            displayLanguage = util::GetSystemPreferredLanguage();
        }

    private:

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

    std::wstring AppModel::GetDisplayLanguage() const
    {
        return m_pAppConfig->displayLanguage;
    }

    bool AppModel::SetDisplayLanguage(const std::wstring & newLanguage)
    {
        auto& availableLanguages = util::GetAvailableLanguages();

        auto iter = std::find_if(availableLanguages.cbegin(), availableLanguages.cend(),
            [&newLanguage](const std::pair<std::wstring, std::wstring>& lang)
        {
            return lang.first == newLanguage;
        });

        assert(iter != availableLanguages.cend());
        if (iter == availableLanguages.cend())
        {
            return false;
        }

        if (db::WriteDBConfig(*m_pUserDB, configKeyLanguage, newLanguage))
        {
            m_pAppConfig->displayLanguage = newLanguage;
            util::SetDisplayLanguage(m_pAppConfig->displayLanguage);

            return true;
        }
        else
        {
            return false;
        }
    }

    filesystem::path AppModel::GetGameExecutablePath() const
    {
        return m_pAppConfig->gameExecutablePath;
    }

    void AppModel::SetGameExecutablePath(const std::wstring& exePath)
    {
        auto oldPath = m_pAppConfig->gameExecutablePath;
        m_pAppConfig->gameExecutablePath = exePath;
        m_pAppConfig->gameDirectory = filesystem::path(exePath).parent_path();

        db::WriteDBConfig(*m_pUserDB, configKeyGameExecutablePath, m_pAppConfig->gameExecutablePath);

        if (exePath != oldPath || oldPath.empty())
        {
            LoadSavedConfigFromGame();
        }

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

        if (iter != m_pAppConfig->serverList.cend() && db::RemoveServerData(serverName))
        {
            db::RemoveAllUsersInServer(serverName);
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

    bool AppModel::SetSensitivityValue(int moveSensitivity, int moveSpeed, int dropSpeed)
    {
        bool ret = util::game::WriteMoveSensitivityConfig(moveSensitivity, moveSpeed, dropSpeed);
        if (ret)
        {
            m_pAppConfig->moveSensitivity = moveSensitivity;
            m_pAppConfig->moveSpeed = moveSpeed;
            m_pAppConfig->softDropSpeed = dropSpeed;
        }

        return ret;
    }

    void AppModel::GetLineClearDelayValue(int & lineClearDelay)
    {
        lineClearDelay = m_pAppConfig->lineClearDelay;
    }

    bool AppModel::SetLineClearDelayValue(int lineClearDelay)
    {
        bool ret = util::game::WriteLineClearDelayConfig(lineClearDelay);
        if (ret)
        {
            m_pAppConfig->lineClearDelay = lineClearDelay;
        }

        return ret;
    }

    std::shared_ptr<SQLite::Database> AppModel::InitUserDB()
    {
        try
        {
            filesystem::path userDBPath = filesystem::path(util::GetWorkDirectory()) / userDBName;
            auto pDB = std::make_shared<SQLite::Database>(util::wstringToUTF8(userDBPath.wstring()), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
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
        if (!langValue.empty())
        {
            m_pAppConfig->displayLanguage = langValue;
        }


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
            return;
            //throw std::runtime_error("Game executable is not found!");
        }

        util::game::ReadMoveSensitivityConfig(m_pAppConfig->moveSensitivity, m_pAppConfig->moveSpeed, m_pAppConfig->softDropSpeed);
        util::game::ReadLineClearDelayConfig(m_pAppConfig->lineClearDelay);
    }
}

