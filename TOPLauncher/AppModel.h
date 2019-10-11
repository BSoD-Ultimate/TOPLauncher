/*
 * stores back-end data & manages db handle
*/
#pragma once
#include <filesystem>

namespace SQLite
{
    class Database;
}


namespace TOPLauncher
{

    struct ServerData
    {
        QString serverName;
        QString serverAddress;
        QString registerURL;
    };

    struct AppConfig;

    struct GameConfig
    {
        // set tetrominos' handling characteristics
        int32_t moveSensitivity;
        int32_t moveSpeed;
        int32_t softDropSpeed;

        // set line-clear delay time
        int32_t lineClearDelay;

        int nextPiecesCount;

        GameConfig()
            : moveSensitivity(45)
            , moveSpeed(15)
            , softDropSpeed(10)
            , lineClearDelay(0)
            , nextPiecesCount(6)
        {
        }
    };


    class AppModel
    {
    private:
        AppModel();
    public:
        ~AppModel();

    public:
        static std::shared_ptr<AppModel> GetInstance();

    public:
        // init
        bool InitModelData(QString* outError = nullptr);

    public:

        std::shared_ptr<SQLite::Database> GetUserDB();

        // app config
        bool LoadAppConfigFromFile();
        bool SaveAppConfigToFile();

        // language
        QString GetDisplayLanguage() const;
        bool SetDisplayLanguage(const QString& newLanguage);

        // game executable
        filesystem::path GetGameExecutablePath() const;
        void SetGameExecutablePath(const std::wstring& exePath);
        filesystem::path GetGameDirectory() const;

        // server list
        bool AddServer(const std::shared_ptr<ServerData> serverData);
        bool RemoveServer(const QString& serverName);
        bool ModifyServer(const QString& serverName, const std::shared_ptr<ServerData> newServerData);
        std::shared_ptr<ServerData> GetServerData(const QString& serverName);
        const std::vector<std::shared_ptr<ServerData>>& GetServerData() const;


        /*
         * game config
        */
        bool IsGameConfigAvailable() const;

        const GameConfig& GetGameConfig() const;
        bool ApplyGameConfig(const GameConfig& newConfig);
        bool ApplyGameConfigTOPDefault();
        bool ApplyGameConfigTOJDefualt();

    private:

        void InitAppConfig();
        void InitDatabase();
        void InitGameConfig();

        bool ApplyTranslator(const QString& newLanguage);

    private:
        std::shared_ptr<SQLite::Database> OpenUserDB();

        // init functions
        // find game executable
        // assume game executable locates in current working directory
        void FindGameExecutablePath();
        // load saved config from game config file
        void LoadSavedConfigFromGame();


    private:
        std::shared_ptr<SQLite::Database> m_pUserDB;

        std::unique_ptr<AppConfig> m_pAppConfig;
        std::unique_ptr<GameConfig> m_pGameConfig;
    };

}

