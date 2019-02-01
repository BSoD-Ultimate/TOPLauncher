#include "stdafx.h"
#include "dbDef.h"

namespace TOPLauncher
{
    namespace db
    {
        namespace table_user
        {
            const std::string t_user = "t_user";

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
    }
}

