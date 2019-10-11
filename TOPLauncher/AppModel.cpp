#include "stdafx.h"
#include "AppModel.h"
#include "LanguageModel.h"
#include "dbDef.h"
#include "dbUser.h"
#include "gameConfigUtil.h"

#include <QtWidgets/QApplication>

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

    struct AppConfig
    {
        // general
        QString displayLanguage;
        filesystem::path gameExecutablePath;
        filesystem::path gameDirectory;

        // server list
        std::vector<std::shared_ptr<ServerData>> serverList;

        AppConfig()
            : displayLanguage(util::GetSystemLanguageName())
            , serverList({
                std::make_shared<ServerData>(ServerData{ "TOP Official Server", "tetrisonline.pl", "http://tetrisonline.pl/top/register.php" })
                })
        {
        }

        bool FromJSON(const std::string& json);
        std::string ToJSON();
    private:

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
                displayLanguage = QString::fromStdString(generalSection["language"].GetString());
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
                        newServerData->serverName = QString::fromStdString(serverObject["name"].GetString());
                    }
                    if (serverObject.HasMember("address") && serverObject["address"].IsString())
                    {
                        newServerData->serverAddress = QString::fromStdString(serverObject["address"].GetString());
                    }
                    if (serverObject.HasMember("registerURL") && serverObject["registerURL"].IsString())
                    {
                        newServerData->registerURL = QString::fromStdString(serverObject["registerURL"].GetString());
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

            auto displayLangUTF8 = displayLanguage.toUtf8();
            generalSection.AddMember("language", Value(displayLangUTF8.constData(), displayLangUTF8.length(), doc.GetAllocator()), doc.GetAllocator());

            auto gameExePathUTF8 = gameExecutablePath.u8string();
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

                auto serverNameUTF8 = serverData->serverName.toUtf8();
                serverObj.AddMember("name", Value(serverNameUTF8.constData(), serverNameUTF8.length(), doc.GetAllocator()), doc.GetAllocator());
                auto serverAddrUTF8 = serverData->serverAddress.toUtf8();
                serverObj.AddMember("address", Value(serverAddrUTF8.constData(), serverAddrUTF8.length(), doc.GetAllocator()), doc.GetAllocator());
                auto registerURLUTF8 = serverData->registerURL.toUtf8();
                serverObj.AddMember("registerURL", Value(registerURLUTF8.constData(), registerURLUTF8.length(), doc.GetAllocator()), doc.GetAllocator());

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
        InitAppConfig();

        // Use English if language currently set is unavailable
        if (!ApplyTranslator(m_pAppConfig->displayLanguage))
        {
            SetDisplayLanguage("en-US");
        }
    }


    AppModel::~AppModel()
    {
    }

    std::shared_ptr<AppModel> AppModel::GetInstance()
    {
        static std::shared_ptr<AppModel> instance(new AppModel());
        return instance;
    }

    bool AppModel::InitModelData(QString * outError)
    {
        try
        {
            InitDatabase();
            InitGameConfig();
            return true;
        }
        catch (const std::runtime_error& e)
        {
            if (outError)
            {
                *outError = QString::fromStdString(std::string(e.what()));
            }
            return false;
        }

    }

    void AppModel::InitAppConfig()
    {
        if (!LoadAppConfigFromFile())
        {
            // use defaults then save
            if (!SaveAppConfigToFile())
            {
                throw std::runtime_error("Unable to load the saved config, or saving default config to file failed.");
            }
        }
    }

    void AppModel::InitDatabase()
    {
        m_pUserDB = OpenUserDB();

        if (!m_pUserDB)
        {
            throw std::runtime_error("Could not open the saved users database.");
        }
    }

    void AppModel::InitGameConfig()
    {
        FindGameExecutablePath();
        LoadSavedConfigFromGame();
    }

    bool AppModel::ApplyTranslator(const QString & newLanguage)
    {
        QString oldLanguage = m_pAppConfig->displayLanguage;
        auto pLangModel = LanguageModel::GetInstance();

        UITranslator* pOldTranslator = nullptr;
        pLangModel->FindTranslator(oldLanguage, &pOldTranslator);

        UITranslator* pNewTranslator = nullptr;
        bool translationFound = pLangModel->FindTranslator(newLanguage, &pNewTranslator);

        if (!translationFound)
        {
            return false;
        }

        if (qApp->installTranslator(pNewTranslator))
        {
            return true;
        }
        else
        {
            qApp->installTranslator(pOldTranslator);
            return false;
        }
    }

    std::shared_ptr<SQLite::Database> AppModel::GetUserDB()
    {
        assert(m_pUserDB);
        if (!m_pUserDB)
        {
            throw SQLite::Exception("Attempt to open a invalid database handle.");
        }
        return m_pUserDB;
    }

    bool AppModel::LoadAppConfigFromFile()
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

    bool AppModel::SaveAppConfigToFile()
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

    QString AppModel::GetDisplayLanguage() const
    {
        return m_pAppConfig->displayLanguage;
    }

    bool AppModel::SetDisplayLanguage(const QString& newLanguage)
    {
        bool ret = ApplyTranslator(newLanguage);

        if (ret)
        {
            m_pAppConfig->displayLanguage = newLanguage;
            SaveAppConfigToFile();
        }

        return ret;
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

        SaveAppConfigToFile();

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
        SaveAppConfigToFile();

        return true;
    }

    bool AppModel::RemoveServer(const QString& serverName)
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
            SaveAppConfigToFile();
            return true;
        }
        else
        {
            return false;
        }
    }

    bool AppModel::ModifyServer(const QString& serverName, const std::shared_ptr<ServerData> newServerData)
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
            SaveAppConfigToFile();
            return true;
        }
    }

    std::shared_ptr<ServerData> AppModel::GetServerData(const QString& serverName)
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

    const GameConfig& AppModel::GetGameConfig() const
    {
        return *m_pGameConfig;
    }

    bool AppModel::ApplyGameConfig(const GameConfig& newConfig)
    {
        bool ret = util::game::WriteGameConfig(newConfig);
        if (ret)
        {
            *m_pGameConfig = newConfig;
        }

        return ret;
    }

    bool AppModel::ApplyGameConfigTOPDefault()
    {
        bool ret = util::game::WriteTOPDefaultConfig();
        if (ret)
        {
            LoadSavedConfigFromGame();
        }

        return ret;
    }

    bool AppModel::ApplyGameConfigTOJDefualt()
    {
        bool ret = util::game::WriteTOJDefaultConfig();
        if (ret)
        {
            LoadSavedConfigFromGame();
        }

        return ret;
    }

    std::shared_ptr<SQLite::Database> AppModel::OpenUserDB()
    {
        try
        {
            filesystem::path userDBPath = filesystem::path(util::GetWorkDirectory().toStdWString()) / userDBName;
            auto pDB = std::make_shared<SQLite::Database>(userDBPath.u8string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            pDB->setBusyTimeout(5000);

            pDB->exec(db::table_user::user::CreateTableSQL().toUtf8().toStdString());

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
            filesystem::path workDir = util::GetWorkDirectory().toUtf8().toStdString();
            filesystem::path gameExePath = workDir / gameExecutableName;

            if (filesystem::exists(gameExePath))
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

        bool readConfigRet = util::game::ReadGameConfig(*m_pGameConfig);

        if (!readConfigRet)
        {
            m_pGameConfig.reset();
        }
    }
}

