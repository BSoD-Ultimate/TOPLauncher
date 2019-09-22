#include "stdafx.h"
#include "dbDef.h"

namespace TOPLauncher
{
    namespace db
    {
        namespace table_user
        {
            const QString t_user = "t_user";

            namespace user
            {
                const QString c_serverName = "c_server_name";
                const QString c_username = "c_username";
                const QString c_password = "c_password";
                const QString c_rememberPassword = "c_remember_password";
                const QString c_lastLoginTime = "c_last_login_time";

                QString CreateTableSQL()
                {
                    static const QString sql = "create table if not exists "
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

