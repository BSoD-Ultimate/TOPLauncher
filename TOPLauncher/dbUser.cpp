#include "stdafx.h"
#include "dbUser.h"

#include "dbDef.h"
#include "AppModel.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace TOPLauncher
{
    namespace db
    {
        DBUserData::DBUserData()
            : savePassword(false)
        {
        }

        std::unique_ptr<SQLite::Statement> DBUserData::GetInsertStatement(SQLite::Database & db) const
        {
            using namespace table_user;

            std::string sql = util::string_format(
                "insert into {} ({}, {}, {}, {}, {}) values "
                " (@serverName, @username, @password, @savePassword, @lastLoginTime) ; ",
                table_user::t_user,
                user::c_serverName, user::c_username, user::c_password, user::c_rememberPassword, user::c_lastLoginTime);

            auto pStmt = std::make_unique<SQLite::Statement>(db, sql);

            pStmt->bind("@serverName", util::wstringToUTF8(serverName));
            pStmt->bind("@username", util::wstringToUTF8(username));
            pStmt->bind("@password", util::wstringToUTF8(password));
            pStmt->bind("@savePassword", savePassword);
            pStmt->bind("@lastLoginTime", util::wstringToUTF8(lastLoginTime.ToTimestamp()));

            return pStmt;
        }

        std::unique_ptr<SQLite::Statement> DBUserData::GetUpdateStatement(SQLite::Database & db) const
        {
            using namespace table_user;

            std::string sql = util::string_format(
                "update {} set "
                " {} = @password, {} = @savePassword, {} = @lastLoginTime "
                " where {} = @serverName and {} = @username ; ",
                table_user::t_user,
                user::c_password, user::c_rememberPassword, user::c_lastLoginTime,
                user::c_serverName, user::c_username);

            auto pStmt = std::make_unique<SQLite::Statement>(db, sql);

            pStmt->bind("@serverName", util::wstringToUTF8(serverName));
            pStmt->bind("@username", util::wstringToUTF8(username));
            pStmt->bind("@password", util::wstringToUTF8(password));
            pStmt->bind("@savePassword", savePassword);
            pStmt->bind("@lastLoginTime", util::wstringToUTF8(lastLoginTime.ToTimestamp()));

            return pStmt;
        }

        static std::shared_ptr<DBUserData> BuildDBUserData(SQLite::Statement& qr)
        {
            using namespace table_user::user;

            auto pUserData = std::make_shared<DBUserData>();

            pUserData->serverName = util::wstringFromUTF8(qr.getColumn(c_serverName.c_str()));
            pUserData->username = util::wstringFromUTF8(qr.getColumn(c_username.c_str()));
            pUserData->password = util::wstringFromUTF8(qr.getColumn(c_password.c_str()));
            pUserData->savePassword = bool(qr.getColumn(c_rememberPassword.c_str()).getInt());
            pUserData->lastLoginTime = util::Time::FromTimestamp(util::wstringFromUTF8(qr.getColumn(c_lastLoginTime.c_str()).getString()));

            return pUserData;
        }

        bool LoadUsersFromServer(const std::wstring & serverName, std::vector<std::shared_ptr<DBUserData>>& userList)
        {
            using namespace table_user;
            userList.clear();

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("select * from {} where {} = @serverName ; ", t_user, user::c_serverName);

                SQLite::Statement qr(*pUserDB, sql);
                qr.bind("@serverName", util::wstringToUTF8(serverName));

                while (qr.executeStep())
                {
                    userList.emplace_back(BuildDBUserData(qr));
                }

                return true;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

        std::shared_ptr<DBUserData> LoadLoginUser(const std::wstring & username, const std::wstring & serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("select * from {} where {} = @serverName and {} = @username ; ", table_user::t_user,
                    c_serverName, c_username);

                SQLite::Statement qr(*pUserDB, sql);
                qr.bind("@serverName", util::wstringToUTF8(serverName));
                qr.bind("@username", util::wstringToUTF8(username));

                if (qr.executeStep())
                {
                    return BuildDBUserData(qr);
                }

                return nullptr;

            }
            catch (const SQLite::Exception&)
            {
                return nullptr;
            }
        }

        std::shared_ptr<DBUserData> LoadLastLoginUser(const std::wstring& serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("select * from {} ", table_user::t_user);
                if (!serverName.empty())
                {
                    sql += util::string_format(" where {} = @serverName ", c_serverName);
                }
                sql += util::string_format(" order by {} desc limit 1 ; ", c_lastLoginTime);

                SQLite::Statement qr(*pUserDB, sql);

                if (!serverName.empty())
                {
                    qr.bind("@serverName", util::wstringToUTF8(serverName));
                }

                if (qr.executeStep())
                {
                    return BuildDBUserData(qr);
                }

                return nullptr;

            }
            catch (const SQLite::Exception&)
            {
                return nullptr;
            }
        }

        bool SaveLoginUser(const DBUserData& userInfo)
        {
            try
            {
                using namespace table_user;

                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sqlCheckExists = util::string_format("select count(*) from {} where {} = @serverName and {} = @username ; ",
                    table_user::t_user, user::c_serverName, user::c_username);

                SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists);
                stmtCheckExists.bind("@serverName", util::wstringToUTF8(userInfo.serverName));
                stmtCheckExists.bind("@username", util::wstringToUTF8(userInfo.username));

                stmtCheckExists.executeStep();

                std::unique_ptr<SQLite::Statement> pUpdateStmt;

                if (stmtCheckExists.getColumn(0).getInt())
                {
                    pUpdateStmt = userInfo.GetUpdateStatement(*pUserDB);
                }
                else
                {
                    pUpdateStmt = userInfo.GetInsertStatement(*pUserDB);
                }

                return pUpdateStmt->exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

        bool SaveLoginUser(std::vector<std::shared_ptr<DBUserData>>& userList)
        {
            try
            {
                using namespace table_user;

                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                SQLite::Transaction tr(*pUserDB);

                for (const std::shared_ptr<DBUserData>& pUser : userList)
                {
                    assert(pUser);

                    std::string sqlCheckExists = util::string_format("select count(*) from {} where {} = @serverName and {} = @username ; ",
                        table_user::t_user, user::c_serverName, user::c_username);

                    SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists);
                    stmtCheckExists.bind("@serverName", util::wstringToUTF8(pUser->serverName));
                    stmtCheckExists.bind("@username", util::wstringToUTF8(pUser->username));

                    stmtCheckExists.executeStep();

                    std::unique_ptr<SQLite::Statement> pUpdateStmt;

                    if (stmtCheckExists.getColumn(0).getInt())
                    {
                        pUpdateStmt = pUser->GetUpdateStatement(*pUserDB);
                    }
                    else
                    {
                        pUpdateStmt = pUser->GetInsertStatement(*pUserDB);
                    }

                    pUpdateStmt->exec();

                }

                tr.commit();

                return true;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

        bool RemoveLoginUser(const std::wstring& serverName, const std::wstring& username)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("delete from {} where {} = @serverName and {} = @username ; ", table_user::t_user,
                    c_serverName, c_username);

                SQLite::Statement stmt(*pUserDB, sql);
                stmt.bind("@serverName", util::wstringToUTF8(serverName));
                stmt.bind("@username", util::wstringToUTF8(username));

                return stmt.exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

        bool RemoveAllUsersInServer(const std::wstring& serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("delete from {} where {} = @serverName ; ", table_user::t_user,
                    c_serverName);

                SQLite::Statement stmt(*pUserDB, sql);
                stmt.bind("@serverName", util::wstringToUTF8(serverName));

                return stmt.exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

    }
}
