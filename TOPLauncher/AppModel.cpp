#include "stdafx.h"
#include "AppModel.h"

#include "dbDef.h"
#include "dbUser.h"

#include "guiUtil.h"
#include "gameConfigUtil.h"

#include <SQLiteCpp/Database.h>

#include <set>
#include <fstream>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/prettywriter.h>


namespace TOPLauncher
{
    static const std::wstring gameExecutableName = L"tetris.exe";
    // app config
    static const std::wstring appConfigFileName = L"config.json";
    // db name 
    static const std::wstring userDBName = L"User.db";

    // reserved server list
    static const std::vector<ServerData> reservedServerList
    {
        { L"TOP Official Server", L"tetrisonline.pl", L"http://tetrisonline.pl/top/register.php" }
    };

    struct AppConfig
    {
        // general
        std::wstring displayLanguage;
        filesystem::path gameExecutablePath;
        filesystem::path gameDirectory;

        // server list
        std::vector<std::shared_ptr<ServerData>> serverList;

        AppConfig()
            : serverList({
                std::make_shared<ServerData>(ServerData{ L"TOP Official Server", L"tetrisonline.pl", L"http://tetrisonline.pl/top/register.php" })
                })
        {
            displayLanguage = util::GetSystemPreferredLanguage();
        }

        bool FromJSON(const std::string& json);
        std::string ToJSON();
    private:

    };

    struct GameConfig
    {
        // set tetrominos' handling characteristics
        int32_t moveSensitivity;
        int32_t moveSpeed;
        int32_t softDropSpeed;

        // set line-clear delay time
        int32_t lineClearDelay;

        GameConfig()
            : moveSensitivity(45)
            , moveSpeed(15)
            , softDropSpeed(10)
            , lineClearDelay(0)
        {
        }
    };

    bool AppConfig::FromJSON(const std::string & json)
    {
        using namespace rapidjson;
        Document doc;
        if (doc.Parse(json.c_str(), json.length()).HasParseError())
        {
            return false;
        }

        if (doc.HasMember("general") && doc["general"].IsObject())
        {
            auto generalSection = doc["general"].GetObject();
            if (generalSection.HasMember("language") && generalSection["language"].IsString())
            {
                displayLanguage = util::wstringFromUTF8(generalSection["language"].GetString());
            }
            if (generalSection.HasMember("game_executable") && generalSection["game_executable"].IsString())
            {
                gameExecutablePath = util::wstringFromUTF8(generalSection["game_executable"].GetString());
                gameDirectory = gameExecutablePath.parent_path();
            }
        }

        if (doc.HasMember("servers") && doc["servers"].IsArray())
        {
            serverList.clear();
            auto serverArray = doc["servers"].GetArray();
            for (size_t i = 0; i < serverArray.Size(); i++)
            {
                if (serverArray[i].IsObject())
                {
                    auto newServerData = std::make_shared<ServerData>();

                    auto serverObject = serverArray[i].GetObject();
                    if (serverObject.HasMember("name") && serverObject["name"].IsString())
                    {
                        newServerData->serverName = util::wstringFromUTF8(serverObject["name"].GetString());
                    }
                    if (serverObject.HasMember("address") && serverObject["address"].IsString())
                    {
                        newServerData->serverAddress = util::wstringFromUTF8(serverObject["address"].GetString());
                    }
                    if (serverObject.HasMember("registerURL") && serverObject["registerURL"].IsString())
                    {
                        newServerData->registerURL = util::wstringFromUTF8(serverObject["registerURL"].GetString());
                    }

                    serverList.emplace_back(newServerData);
                }
            }
        }

        return true;
    }
    std::string AppConfig::ToJSON()
    {
        using namespace rapidjson;

        Document doc;
        doc.SetObject();

        // "general"
        {
            rapidjson::Value generalSection;
            generalSection.SetObject();

            auto displayLangUTF8 = util::wstringToUTF8(displayLanguage);
            generalSection.AddMember("language", Value(displayLangUTF8.c_str(), displayLangUTF8.length(), doc.GetAllocator()), doc.GetAllocator());

            auto gameExePathUTF8 = util::wstringToUTF8(gameExecutablePath.wstring());
            generalSection.AddMember("game_executable", Value(gameExePathUTF8.c_str(), gameExePathUTF8.length(), doc.GetAllocator()), doc.GetAllocator());

            doc.AddMember("general", generalSection, doc.GetAllocator());
        }

        // "servers"
        {
            rapidjson::Value serverArray;
            serverArray.SetArray();

            for (const auto& serverData : serverList)
            {
                rapidjson::Value serverObj;
                serverObj.SetObject();

                auto serverNameUTF8 = util::wstringToUTF8(serverData->serverName);
                serverObj.AddMember("name", Value(serverNameUTF8.c_str(), serverNameUTF8.length(), doc.GetAllocator()), doc.GetAllocator());
                auto serverAddrUTF8 = util::wstringToUTF8(serverData->serverAddress);
                serverObj.AddMember("address", Value(serverAddrUTF8.c_str(), serverAddrUTF8.length(), doc.GetAllocator()), doc.GetAllocator());
                auto registerURLUTF8 = util::wstringToUTF8(serverData->registerURL);
                serverObj.AddMember("registerURL", Value(registerURLUTF8.c_str(), registerURLUTF8.length(), doc.GetAllocator()), doc.GetAllocator());

                serverArray.PushBack(serverObj, doc.GetAllocator());
            }

            doc.AddMember("servers", serverArray, doc.GetAllocator());
        }

        rapidjson::StringBuffer jsonBuf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(jsonBuf);
        doc.Accept(writer);

        return std::string(jsonBuf.GetString(), jsonBuf.GetLength());
    }

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

    bool AppModel::InitAppConfig()
    {
        m_pUserDB = InitUserDB();

        if (!m_pUserDB)
        {
            return false;
        }

        // TODO: load saved config from json
        if (!LoadAppConfig())
        {
            // use defaults then save
            SaveAppConfig();
        }

        return true;
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

    bool AppModel::LoadAppConfig()
    {
        std::wstring configFilePath = appConfigFileName;
        if (!filesystem::exists(configFilePath))
        {
            return false;
        }

        std::ifstream configFile(configFilePath, std::ios_base::in | std::ios_base::binary);
        if (configFile.bad())
        {
            return false;
        }

        uint64_t configSize = filesystem::file_size(configFilePath);
        std::unique_ptr<char[]> configBuf(new char[configSize]);
        configFile.read(configBuf.get(), configSize);

        configFile.close();

        return m_pAppConfig->FromJSON(std::string(configBuf.get(), configSize));
    }

    bool AppModel::SaveAppConfig()
    {
        std::wstring configFilePath = appConfigFileName;

        std::ofstream configFile(configFilePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        if (configFile.bad())
        {
            return false;
        }
        std::string appConfigJSON = m_pAppConfig->ToJSON();

        configFile.write(appConfigJSON.c_str(), appConfigJSON.length());

        configFile.close();

        return true;
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

        m_pAppConfig->displayLanguage = newLanguage;
        util::SetDisplayLanguage(m_pAppConfig->displayLanguage);

        return true;
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

        if (exePath != oldPath || oldPath.empty())
        {
            LoadSavedConfigFromGame();
        }

    }

    filesystem::path AppModel::GetGameDirectory() const
    {
        return m_pAppConfig->gameDirectory;
    }

    bool AppModel::AddServer(const std::shared_ptr<ServerData> serverData)
    {
        auto pData = GetServerData(serverData->serverName);

        if (pData)
        {
            return false;
        }

        m_pAppConfig->serverList.push_back(serverData);

        return true;
    }

    bool AppModel::RemoveServer(const std::wstring& serverName)
    {
        auto iter = std::find_if(m_pAppConfig->serverList.cbegin(), m_pAppConfig->serverList.cend(),
            [serverName](const std::shared_ptr<ServerData>& data)
        {
            return data->serverName == serverName;
        });

        if (iter != m_pAppConfig->serverList.cend())
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

    bool AppModel::ModifyServer(const std::wstring & serverName, const std::shared_ptr<ServerData> newServerData)
    {
        auto iter = std::find_if(m_pAppConfig->serverList.begin(), m_pAppConfig->serverList.end(),
            [serverName](const std::shared_ptr<ServerData>& data)
        {
            return data->serverName == serverName;
        });

        if (iter == m_pAppConfig->serverList.cend())
        {
            return false;
        }
        else
        {
            *iter = newServerData;
            return true;
        }
    }

    std::shared_ptr<ServerData> AppModel::GetServerData(const std::wstring & serverName)
    {
        auto iter = std::find_if(m_pAppConfig->serverList.cbegin(), m_pAppConfig->serverList.cend(),
            [serverName](const std::shared_ptr<ServerData>& data)
        {
            return data->serverName == serverName;
        });
        if (iter != m_pAppConfig->serverList.cend())
        {
            return *iter;
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<ServerData>>& AppModel::GetServerData() const
    {
        return m_pAppConfig->serverList;
    }

    bool AppModel::IsGameConfigAvailable() const
    {
        return m_pGameConfig.operator bool();
    }

    void AppModel::GetSensitivityValue(int & moveSensitivity, int & moveSpeed, int & dropSpeed)
    {
        moveSensitivity = m_pGameConfig->moveSensitivity;
        moveSpeed = m_pGameConfig->moveSpeed;
        dropSpeed = m_pGameConfig->softDropSpeed;
    }

    bool AppModel::SetSensitivityValue(int moveSensitivity, int moveSpeed, int dropSpeed)
    {
        bool ret = util::game::WriteMoveSensitivityConfig(moveSensitivity, moveSpeed, dropSpeed);
        if (ret)
        {
            m_pGameConfig->moveSensitivity = moveSensitivity;
            m_pGameConfig->moveSpeed = moveSpeed;
            m_pGameConfig->softDropSpeed = dropSpeed;
        }

        return ret;
    }

    void AppModel::GetLineClearDelayValue(int & lineClearDelay)
    {
        lineClearDelay = m_pGameConfig->lineClearDelay;
    }

    bool AppModel::SetLineClearDelayValue(int lineClearDelay)
    {
        bool ret = util::game::WriteLineClearDelayConfig(lineClearDelay);
        if (ret)
        {
            m_pGameConfig->lineClearDelay = lineClearDelay;
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

            pDB->exec(db::table_user::user::CreateTableSQL());

            return pDB;
        }
        catch (const SQLite::Exception& e)
        {
            return nullptr;
        }

        return nullptr;
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
            m_pGameConfig.reset();
            return;
        }

        m_pGameConfig.reset(new GameConfig());

        util::game::ReadMoveSensitivityConfig(m_pGameConfig->moveSensitivity, m_pGameConfig->moveSpeed, m_pGameConfig->softDropSpeed);
        util::game::ReadLineClearDelayConfig(m_pGameConfig->lineClearDelay);
    }
}

