/*
* definition of the structure of the database used to store saved servers/users/configs
*/

#pragma once

namespace TOPLauncher
{
    namespace db
    {
        namespace table_user
        {
            extern const std::string t_user;

            namespace user
            {
                extern const std::string c_serverName;
                extern const std::string c_username;
                extern const std::string c_password;
                extern const std::string c_rememberPassword;
                extern const std::string c_lastLoginTime;

                std::string CreateTableSQL();
            }

        }
    }
}
