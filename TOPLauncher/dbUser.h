/*
 * manages saved users stored in DB
*/

#pragma once

#include <QString>

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
            QString serverName;
            QString username;
            QString password;
            bool savePassword;
            util::Time lastLoginTime;

            DBUserData();

            std::unique_ptr<SQLite::Statement> GetInsertStatement(SQLite::Database& db) const;
            std::unique_ptr<SQLite::Statement> GetUpdateStatement(SQLite::Database& db) const;
        };

        bool LoadUsersFromServer(const QString& serverName, std::vector<std::shared_ptr<DBUserData>>& userList);

        std::shared_ptr<DBUserData> LoadLoginUser(const QString& username, const QString& serverName);
        std::shared_ptr<DBUserData> LoadLastLoginUser(const QString& serverName = "");

        bool SaveLoginUser(const DBUserData& userInfo);
        bool SaveLoginUser(std::vector<std::shared_ptr<DBUserData>>& userList);

        bool RemoveLoginUser(const QString& serverName, const QString& username);

        bool RemoveAllUsersInServer(const QString& serverName);
    }

}


