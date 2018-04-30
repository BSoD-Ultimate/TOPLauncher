/* 
 * manages saved server addresses stored in DB
*/
#pragma once

namespace SQLite
{
    class Database;
    class Statement;
}

namespace TOPLauncher
{
    namespace db
    {
        struct DBServerData
        {
            std::wstring serverName;
            std::wstring serverAddress;

            std::unique_ptr<SQLite::Statement> GetInsertStatement(SQLite::Database& db) const;
            std::unique_ptr<SQLite::Statement> GetUpdateStatement(SQLite::Database& db) const;
        };

        bool LoadAllServers(std::vector<std::shared_ptr<DBServerData>>& serverList);

        bool SaveServerData(const DBServerData& serverData);
        bool SaveServerData(const std::vector<std::shared_ptr<DBServerData>>& serverList);

        bool RemoveServerData(const std::wstring& serverName);

    }

}