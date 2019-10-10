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
            // keypress sensitivity
            bool ReadMoveSensitivityConfig(int& moveSensitivity, int& moveSpeed, int& softDropSpeed);
            bool WriteMoveSensitivityConfig(int moveSensitivity, int moveSpeed, int softDropSpeed);

            bool ReadLineClearDelayConfig(int& lineClearDelay);
            bool WriteLineClearDelayConfig(int lineClearDelay);
        }
    }
}
