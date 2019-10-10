/*
* utilities reading / writing config values from the game folder
*/
#pragma once

#include <functional>

namespace TOPLauncher
{
    namespace util
    {
        namespace game
        {
            struct GameConfig
            {
                // set tetrominos' handling characteristics
                int32_t moveSensitivity;
                int32_t moveSpeed;
                int32_t softDropSpeed;

                // set line-clear delay time
                int32_t lineClearDelay;

                int nextPiecesCount;

                GameConfig()
                    : moveSensitivity(45)
                    , moveSpeed(15)
                    , softDropSpeed(10)
                    , lineClearDelay(0)
                    , nextPiecesCount(6)
                {
                }
            };

            // gameConfig reading/writing
            bool ReadGameConfig(GameConfig& out);
            bool WriteGameConfig(const GameConfig& config);
        }
    }
}
