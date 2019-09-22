/*
* definition of the structure of the database used to store saved servers/users/configs
*/

#pragma once

#include <QString>

namespace TOPLauncher
{
    namespace db
    {
        namespace table_user
        {
            extern const QString t_user;

            namespace user
            {
                extern const QString c_serverName;
                extern const QString c_username;
                extern const QString c_password;
                extern const QString c_rememberPassword;
                extern const QString c_lastLoginTime;

                QString CreateTableSQL();
            }

        }
    }
}
