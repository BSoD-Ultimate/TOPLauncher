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
            using ArchiveProcessCallback = std::function<bool(const std::wstring&, size_t, int, int)>;

            bool UnpackSJEJHHArchive(const std::wstring& filePath, const std::wstring& extractDir, std::string& internalFolderName, ArchiveProcessCallback callback = nullptr);
            bool PackSJEJHHArchive(const std::wstring& packDir, const std::wstring& saveFilePath, const std::string& internalFolderName, ArchiveProcessCallback callback = nullptr);
            // keypress sensitivity
            bool ReadMoveSensitivityConfig(int& moveSensitivity, int& moveSpeed, int& softDropSpeed);
            bool WriteMoveSensitivityConfig(int moveSensitivity, int moveSpeed, int softDropSpeed);

            bool ReadLineClearDelayConfig(int& lineClearDelay);
            bool WriteLineClearDelayConfig(int lineClearDelay);
        }
    }
}
