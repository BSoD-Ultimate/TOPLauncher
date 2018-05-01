/*
* utilities reading / writing config values from the game folder
*/
#pragma once

namespace TOPLauncher
{
    namespace util
    {

        // keypress sensitivity
        bool ReadMoveSensitivityConfig(int& moveSensitivity, int& moveSpeed, int& softDropSpeed);
        bool WriteMoveSensitivityConfig(int moveSensitivity, int moveSpeed, int softDropSpeed);
    }
}
