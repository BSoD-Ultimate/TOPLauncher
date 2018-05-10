#include "stdafx.h"
#include "dbDef.h"

namespace TOPLauncher
{
    namespace db
    {
        namespace table_user
        {
            const std::string t_server = "t_server";
            const std::string t_user = "t_user";

            namespace server
            {
                const std::string c_serverName = "c_server_name";
                const std::string c_host = "c_host";
                const std::string c_register_url = "c_register_url";

                std::string CreateTableSQL()
                {
                    static const std::string sql = "create table if not exists "
                        + t_server + " ( " 
                        + c_serverName + " nvarchar(200) not null, "
                        + c_host + " text not null, "
                        + c_register_url + " text, "
                        + " primary key ( " + c_serverName + " ) "
                        + " ); "; 
                    return sql;
                }

            }

            namespace user
            {
                const std::string c_serverName = "c_server_name";
                const std::string c_username = "c_username";
                const std::string c_password = "c_password";
                const std::string c_rememberPassword = "c_remember_password";
                const std::string c_lastLoginTime = "c_last_login_time";

                std::string CreateTableSQL()
                {
                    static const std::string sql = "create table if not exists "
                        + t_user + " ( "
                        + c_serverName + " nvarchar(200) not null, "
                        + c_username + " text not null, "
                        + c_password + " text, "
                        + c_rememberPassword + " integer, "
                        + c_lastLoginTime + " timestamp, "
                        + " primary key ( " + c_serverName + " , " + c_username + " ) "
                        + " ); ";
                    return sql;
                }
            }
            
        }

        namespace table_public
        {
            const std::string t_config = "t_config";
            
            namespace config
            {
                const std::string c_key = "c_key";
                const std::string c_value = "c_value";

                std::string CreateTableSQL()
                {
                    static const std::string sql = "create table if not exists "
                        + t_config + " ( "
                        + c_key + " nvarchar(200) not null, "
                        + c_value + " text, "
                        + " primary key ( " + c_key + " ) "
                        + " ); ";
                    return sql;
                }
            }
        }
    }
}

