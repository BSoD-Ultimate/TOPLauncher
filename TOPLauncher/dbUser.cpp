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

            QString sql = QString(
                "insert into %1 (%2, %3, %4, %5, %6) values "
                " (@serverName, @username, @password, @savePassword, @lastLoginTime) ; ")
                .arg(
                    table_user::t_user,
                    user::c_serverName, user::c_username, user::c_password, user::c_rememberPassword, user::c_lastLoginTime
                );
            
            auto pStmt = std::make_unique<SQLite::Statement>(db, sql.toUtf8().toStdString());

            pStmt->bind("@serverName", serverName.toUtf8().toStdString());
            pStmt->bind("@username", username.toUtf8().toStdString());
            pStmt->bind("@password", password.toUtf8().toStdString());
            pStmt->bind("@savePassword", savePassword);
            pStmt->bind("@lastLoginTime", util::wstringToUTF8(lastLoginTime.ToTimestamp()));

            return pStmt;
        }

        std::unique_ptr<SQLite::Statement> DBUserData::GetUpdateStatement(SQLite::Database & db) const
        {
            using namespace table_user;

            QString sql = QString(
                "update %1 set "
                " %2 = @password, %3 = @savePassword, %4 = @lastLoginTime "
                " where %5 = @serverName and %6 = @username ; ")
                .arg(
                    table_user::t_user,
                    user::c_password, user::c_rememberPassword, user::c_lastLoginTime,
                    user::c_serverName, user::c_username
                );

            auto pStmt = std::make_unique<SQLite::Statement>(db, sql.toUtf8().toStdString());

            pStmt->bind("@serverName", serverName.toUtf8().toStdString());
            pStmt->bind("@username", username.toUtf8().toStdString());
            pStmt->bind("@password", password.toUtf8().toStdString());
            pStmt->bind("@savePassword", savePassword);
            pStmt->bind("@lastLoginTime", util::wstringToUTF8(lastLoginTime.ToTimestamp()));

            return pStmt;
        }

        static std::shared_ptr<DBUserData> BuildDBUserData(SQLite::Statement& qr)
        {
            using namespace table_user::user;

            auto pUserData = std::make_shared<DBUserData>();

            pUserData->serverName = QString::fromStdString(qr.getColumn(c_serverName.toUtf8().constData()).getString());
            pUserData->username = QString::fromStdString(qr.getColumn(c_username.toUtf8().constData()));
            pUserData->password = QString::fromStdString(qr.getColumn(c_password.toUtf8().constData()));
            pUserData->savePassword = bool(qr.getColumn(c_rememberPassword.toUtf8().constData()).getInt());
            pUserData->lastLoginTime = util::Time::FromTimestamp(util::wstringFromUTF8(qr.getColumn(c_lastLoginTime.toUtf8().constData()).getString()));

            return pUserData;
        }

        bool LoadUsersFromServer(const QString& serverName, std::vector<std::shared_ptr<DBUserData>>& userList)
        {
            using namespace table_user;
            userList.clear();

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                QString sql = QString("select * from %1 where %2 = @serverName ; ").arg(t_user, user::c_serverName);

                SQLite::Statement qr(*pUserDB, sql.toUtf8().toStdString());
                qr.bind("@serverName", serverName.toUtf8().toStdString());

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

        std::shared_ptr<DBUserData> LoadLoginUser(const QString& username, const QString& serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                QString sql = QString("select * from %1 where %2 = @serverName and %3 = @username ; ")
                    .arg(
                    table_user::t_user,
                    c_serverName, c_username
                    );

                SQLite::Statement qr(*pUserDB, sql.toUtf8().toStdString());
                qr.bind("@serverName", serverName.toUtf8().toStdString());
                qr.bind("@username", username.toUtf8().toStdString());

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

        std::shared_ptr<DBUserData> LoadLastLoginUser(const QString& serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                QString sql = QString("select * from %1 ").arg(table_user::t_user);
                if (!serverName.isEmpty())
                {
                    sql += QString(" where %1 = @serverName ").arg(c_serverName);
                }
                sql += QString(" order by %1 desc limit 1 ; ").arg(c_lastLoginTime);

                SQLite::Statement qr(*pUserDB, sql.toUtf8().toStdString());

                if (!serverName.isEmpty())
                {
                    qr.bind("@serverName", serverName.toUtf8().toStdString());
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

                QString sqlCheckExists = QString("select count(*) from %1 where %2 = @serverName and %3 = @username ; ")
                    .arg(
                        table_user::t_user,
                        user::c_serverName, user::c_username
                    );

                SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists.toUtf8().toStdString());
                stmtCheckExists.bind("@serverName", userInfo.serverName.toUtf8().toStdString());
                stmtCheckExists.bind("@username", userInfo.username.toUtf8().toStdString());

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

                    QString sqlCheckExists = QString("select count(*) from %1 where %2 = @serverName and %3 = @username ; ")
                        .arg(
                            table_user::t_user,
                            user::c_serverName, user::c_username
                        );

                    SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists.toUtf8().toStdString());
                    stmtCheckExists.bind("@serverName", pUser->serverName.toUtf8().toStdString());
                    stmtCheckExists.bind("@username", pUser->username.toUtf8().toStdString());

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

        bool RemoveLoginUser(const QString& serverName, const QString& username)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                QString sql = QString("delete from %1 where %2 = @serverName and %3 = @username ; ")
                    .arg(
                        table_user::t_user,
                        c_serverName, c_username
                    );

                SQLite::Statement stmt(*pUserDB, sql.toUtf8().toStdString());
                stmt.bind("@serverName", serverName.toUtf8().toStdString());
                stmt.bind("@username", username.toUtf8().toStdString());

                return stmt.exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

        bool RemoveAllUsersInServer(const QString& serverName)
        {
            using namespace table_user::user;

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                QString sql = QString("delete from %1 where %2 = @serverName ; ")
                    .arg(
                        table_user::t_user,
                        c_serverName
                    );

                SQLite::Statement stmt(*pUserDB, sql.toUtf8().toStdString());
                stmt.bind("@serverName", serverName.toUtf8().toStdString());

                return stmt.exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }

    }
}
