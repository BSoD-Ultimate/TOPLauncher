#include "stdafx.h"
#include "dbServer.h"

#include "dbDef.h"
#include "AppModel.h"
#include <SQLiteCpp/SQLiteCpp.h>

namespace TOPLauncher
{
    namespace db
    {
        std::unique_ptr<SQLite::Statement> DBServerData::GetInsertStatement(SQLite::Database& db) const
        {
            using namespace table_user;

            std::string sql = util::string_format("insert into {} ({}, {}, {}) values (@serverName, @serverHost, @serverRegisterURL) ; ",
                table_user::t_server, server::c_serverName, server::c_host, server::c_register_url);

            auto pStmt = std::make_unique<SQLite::Statement>(db, sql);

            pStmt->bind("@serverName", util::wstringToUTF8(serverName));
            pStmt->bind("@serverHost", util::wstringToUTF8(serverAddress));
            pStmt->bind("@serverRegisterURL", util::wstringToUTF8(registerURL));

            return pStmt;
        }
        std::unique_ptr<SQLite::Statement> DBServerData::GetUpdateStatement(SQLite::Database& db) const
        {
            using namespace table_user;

            std::string sql = util::string_format("update {} set {} = @serverHost, {} = @serverRegisterURL where {} = @serverName ; ",
                table_user::t_server, server::c_host, server::c_register_url, server::c_serverName);

            auto pStmt = std::make_unique<SQLite::Statement>(db, sql);

            pStmt->bind("@serverName", util::wstringToUTF8(serverName));
            pStmt->bind("@serverHost", util::wstringToUTF8(serverAddress));
            pStmt->bind("@serverRegisterURL", util::wstringToUTF8(registerURL));

            return pStmt;
        }

        static std::shared_ptr<DBServerData> BuildServerData(SQLite::Statement& qr)
        {
            using namespace table_user::server;

            auto serverData = std::make_shared<DBServerData>();
            serverData->serverName = util::wstringFromUTF8(qr.getColumn(c_serverName.c_str()).getString());
            serverData->serverAddress = util::wstringFromUTF8(qr.getColumn(c_host.c_str()).getString());
            serverData->registerURL = util::wstringFromUTF8(qr.getColumn(c_register_url.c_str()).getString());

            return serverData;
        }

        bool LoadAllServers(std::vector<std::shared_ptr<DBServerData>>& serverList)
        {
            serverList.clear();

            try
            {
                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("select * from {}", table_user::t_server);

                SQLite::Statement qr(*pUserDB, sql);

                while (qr.executeStep())
                {
                    serverList.emplace_back(BuildServerData(qr));
                }

                return true;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }
        bool SaveServerData(const DBServerData& serverData)
        {
            try
            {
                using namespace table_user;

                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sqlCheckExists = util::string_format("select count(*) from {} where {} = @serverName", 
                    table_user::t_server, server::c_serverName);

                SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists);
                stmtCheckExists.bind("@serverName", util::wstringToUTF8(serverData.serverName));
                
                stmtCheckExists.executeStep();

                std::unique_ptr<SQLite::Statement> pUpdateStmt;

                if (stmtCheckExists.getColumn(0).getInt())
                {
                    pUpdateStmt = serverData.GetUpdateStatement(*pUserDB);
                }
                else
                {
                    pUpdateStmt = serverData.GetInsertStatement(*pUserDB);
                }

                return pUpdateStmt->exec() > 0;

            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }
        bool SaveServerData(const std::vector<std::shared_ptr<DBServerData>>& serverList)
        {
            try
            {
                using namespace table_user;

                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                SQLite::Transaction tr(*pUserDB);

                for (const std::shared_ptr<DBServerData>& serverData : serverList)
                {
                    assert(serverData);

                    std::string sqlCheckExists = util::string_format("select count(*) from {} where {} = @serverName",
                        table_user::t_server, server::c_serverName);

                    SQLite::Statement stmtCheckExists(*pUserDB, sqlCheckExists);
                    stmtCheckExists.bind("@serverName", util::wstringToUTF8(serverData->serverName));

                    stmtCheckExists.executeStep();

                    std::unique_ptr<SQLite::Statement> pSaveStmt;

                    if (stmtCheckExists.getColumn(0).getInt())
                    {
                        pSaveStmt = serverData->GetUpdateStatement(*pUserDB);
                    }
                    else
                    {
                        pSaveStmt = serverData->GetInsertStatement(*pUserDB);
                    }

                    pSaveStmt->exec();
                }

                tr.commit();

                return true;
            }
            catch (const SQLite::Exception&)
            {
                return false;
            }
        }
        bool RemoveServerData(const std::wstring & serverName)
        {
            try
            {
                using namespace table_user;

                auto pUserDB = AppModel::GetInstance()->GetUserDB();

                std::string sql = util::string_format("delete from {} where {} = @serverName ; ",
                    table_user::t_server, server::c_serverName);

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


