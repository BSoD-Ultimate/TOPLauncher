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

    struct DBServerData
    {
        std::wstring serverName;
        std::wstring serverAddress;
        std::wstring registerURL;
    };

    struct AppConfig;

    class AppModel
    {
    private:
        AppModel();
    public:
        ~AppModel();

    public:
        static std::shared_ptr<AppModel> GetInstance();

        // init mothods
        bool InitSavedData();
        bool InitGameConfig();

        std::shared_ptr<SQLite::Database> GetUserDB();

        // language

        std::wstring GetDisplayLanguage() const;
        bool SetDisplayLanguage(const std::wstring& newLanguage);

        // game executable
        filesystem::path GetGameExecutablePath() const;
        void SetGameExecutablePath(const std::wstring& exePath);
        filesystem::path GetGameDirectory() const;

        // server list
        bool AddServer(const DBServerData& serverData);
        bool RemoveServer(const std::wstring& serverName);
        bool ModifyServer(const std::wstring& serverName, const DBServerData& newServerData);
        DBServerData GetServerData(const std::wstring& serverName);
        const std::vector<DBServerData>& GetServerData() const;

        // game config
        bool IsGameConfigAvailable() const;

        void GetSensitivityValue(int& moveSensitivity, int& moveSpeed, int& dropSpeed);
        bool SetSensitivityValue(int moveSensitivity, int moveSpeed, int dropSpeed);

        void GetLineClearDelayValue(int& lineClearDelay);
        bool SetLineClearDelayValue(int lineClearDelay);

    private:
        std::shared_ptr<SQLite::Database> InitUserDB();

        // init functions
        // find game executable
        // assume game executable locates in current working directory
        void FindGameExecutablePath();
        // load saved config from game config file
        void LoadSavedConfigFromGame();


    private:
        std::shared_ptr<SQLite::Database> m_pUserDB;

        std::unique_ptr<AppConfig> m_pAppConfig;
    };

}

