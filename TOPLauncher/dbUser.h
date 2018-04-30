/*
 * manages saved users stored in DB
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
        struct DBUserData
        {
            std::wstring serverName;
            std::wstring username;
            std::wstring password;
            bool savePassword;
            util::Time lastLoginTime;

            DBUserData();

            std::unique_ptr<SQLite::Statement> GetInsertStatement(SQLite::Database& db) const;
            std::unique_ptr<SQLite::Statement> GetUpdateStatement(SQLite::Database& db) const;
        };

        bool LoadUsersFromServer(const std::wstring& serverName, std::vector<std::shared_ptr<DBUserData>>& userList);

        std::shared_ptr<DBUserData> LoadLoginUser(const std::wstring& username, const std::wstring& serverName);
        std::shared_ptr<DBUserData> LoadLastLoginUser(const std::wstring& serverName = L"");

        bool SaveLoginUser(const DBUserData& userInfo);
        bool SaveLoginUser(std::vector<std::shared_ptr<DBUserData>>& userList);

        bool RemoveLoginUser(const std::wstring& serverName, const std::wstring& username);

        bool RemoveAllUsersInServer(const std::wstring& serverName);
    }

}


