#include "stdafx.h"
#include "dbConfig.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include "dbDef.h"

namespace TOPLauncher
{
    namespace db
    {
        std::wstring ReadDBConfig(SQLite::Database& db, const std::wstring& key)
        {
            using namespace db::table_public;

            std::string sqlRead =
                util::string_format("select {}, {} from {} where {} = @key;",
                    config::c_key, config::c_value, table_public::t_config, config::c_key);
            try
            {
                SQLite::Statement qr(db, sqlRead);
                qr.bind("@key", util::wstringToUTF8(key));

                if (qr.executeStep())
                {
                    return util::wstringFromUTF8(qr.getColumn(config::c_value.c_str()).getString());
                }
                else
                {
                    return L"";
                }
            }
            catch (const SQLite::Exception&)
            {
                return L"";
            }
            return std::wstring();
        }

        bool WriteDBConfig(SQLite::Database& db, const std::wstring& key, const std::wstring& value)
        {
            using namespace db;
            using namespace db::table_public;

            try
            {
                std::string sqlCheckExists =
                    util::string_format("select count(1) from {} where {} = @key;", table_public::t_config, config::c_key);
                SQLite::Statement stmtCheckExists(db, sqlCheckExists);
                stmtCheckExists.bind("@key", util::wstringToUTF8(key));
                stmtCheckExists.executeStep();
                bool configExists = stmtCheckExists.getColumn(0).getInt() > 0;

                std::unique_ptr<SQLite::Statement> pStmt;

                if (!configExists)
                {
                    std::string saveSQL = util::string_format("insert into {} ({}, {}) values (@key, @value);",
                        table_public::t_config, config::c_key, config::c_value);
                    pStmt.reset(new SQLite::Statement(db, saveSQL));
                }
                else
                {
                    std::string saveSQL = util::string_format("update {} set {} = @value where {} = @key ;",
                        table_public::t_config, config::c_value, config::c_key);
                    pStmt.reset(new SQLite::Statement(db, saveSQL));

                }

                pStmt->bind("@key", util::wstringToUTF8(key));
                pStmt->bind("@value", util::wstringToUTF8(value));

                return pStmt->exec() > 0;
            }
            catch (const SQLite::Exception&)
            {
                return false;
            }


            return true;
        }
    }
}


