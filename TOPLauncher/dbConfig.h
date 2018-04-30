/*
 * read / write config value to DB
*/
#pragma once

namespace SQLite
{
    class Database;
}

namespace TOPLauncher
{
    namespace db
    {
        std::wstring ReadDBConfig(SQLite::Database& db, const std::wstring& key);
        bool WriteDBConfig(SQLite::Database& db, const std::wstring& key, const std::wstring& value);
    }
}