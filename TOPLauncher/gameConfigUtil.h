/*
* utilities reading / writing config values from the game folder
*/
#pragma once

#include <functional>

namespace TOPLauncher
{
    struct GameConfig;
    namespace util
    {
        namespace game
        {
            // gameConfig reading/writing
            bool ReadGameConfig(GameConfig& out);
            bool WriteGameConfig(const GameConfig& config);

            bool WriteTOPDefaultConfig();
            bool WriteTOJDefaultConfig();
        }
    }
}
