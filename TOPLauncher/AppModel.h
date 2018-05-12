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
    namespace db
    {
        struct DBServerData;
    }

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
        bool AddServer(const db::DBServerData& serverData);
        bool RemoveServer(const std::wstring& serverName);
        bool ModifyServer(const std::wstring& serverName, const db::DBServerData& newServerData);
        std::shared_ptr<db::DBServerData> GetServerData(const std::wstring& serverName);
        const std::vector<std::shared_ptr<db::DBServerData>>& GetServerData() const;
        bool IsReservedServer(const std::wstring& serverName) const;

        // game config
        bool IsGameConfigAvailable() const;

        void GetSensitivityValue(int& moveSensitivity, int& moveSpeed, int& dropSpeed);
        bool SetSensitivityValue(int moveSensitivity, int moveSpeed, int dropSpeed);

    private:
        std::shared_ptr<SQLite::Database> InitUserDB();

        // init functions
        // load saved config stored in DB
        void LoadSavedConfigFromDB();
        // load saved servers stored in DB
        void LoadSavedServers();
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

