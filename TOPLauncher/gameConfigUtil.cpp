#include "stdafx.h"
#include "gameConfigUtil.h"

#include "AppModel.h"
#include "Environment.h"

#include "SjeJhhUtil.h"
#include "INIReader.h"

#include <algorithm>
#include <utility>

namespace TOPLauncher
{
    namespace util
    {
        namespace game
        {
            static bool UnpackSJEJHHArchive(const std::wstring& filePath, const std::wstring& extractDir, std::string& internalFolderName)
            {
                filesystem::path archivePath = filePath;

                sjejhh_unpack_context* pArchive = sjejhh_unpack_open(filePath.c_str());

                if (!pArchive)
                {
                    return false;
                }

                // get global info
                sjejhh_unpack_global_info gi = { 0 };
                sjejhh_unpack_get_global_info(pArchive, &gi);

                internalFolderName.clear();
                internalFolderName.assign(gi.internalFolderName, gi.internalFolderNameLength);

                filesystem::create_directories(extractDir);

                for (size_t i = 0; i < gi.fileCount; i++)
                {
                    sjejhh_unpack_file_info curFileInfo = { 0 };
                    sjejhh_unpack_get_current_file_info(pArchive, &curFileInfo);

                    filesystem::path extractFileName = extractDir;
                    extractFileName /= std::wstring(curFileInfo.filename, curFileInfo.filenameLength);
                    FILE* fp = _wfopen(extractFileName.c_str(), L"wb");

                    const size_t bufSize = 1000;
                    std::unique_ptr<char[]> readBuf(new char[bufSize]);
                    size_t readBytes = 0;
                    size_t bytesRemaining = 0;

                    while (sjejhh_unpack_read_current_file(pArchive, readBuf.get(), bufSize, &readBytes, &bytesRemaining) != SJEJHH_UNPACK_EOF)
                    {
                        if (!curFileInfo.isEncrypted)
                        {
                            fwrite(readBuf.get(), 1, readBytes, fp);
                        }
                        else
                        {
                            bool isEnd = bytesRemaining == 0;

                            int decryptLength = sjejhh_unpack_decrypt_read_data(pArchive, readBuf.get(), readBytes, NULL, 0);
                            std::unique_ptr<char[]> decryptedData(new char[decryptLength]);
                            sjejhh_unpack_decrypt_read_data(pArchive, readBuf.get(), readBytes, decryptedData.get(), decryptLength);
                            fwrite(decryptedData.get(), 1, decryptLength, fp);
                            fflush(fp);
                        }
                    }

                    fclose(fp);

                    sjejhh_unpack_goto_next_file(pArchive);

                }

                sjejhh_unpack_close(pArchive);

                return true;
            }

            static bool PackSJEJHHArchive(const std::wstring& packDir, const std::wstring& saveFilePath, const std::string& internalFolderName)
            {
                std::error_code code;
                filesystem::directory_iterator dirIter(packDir, code);

                sjejhh_pack_context* pPackContext = sjejhh_pack_create_file(internalFolderName.c_str(), saveFilePath.c_str());

                while (dirIter != filesystem::end(dirIter))
                {
                    std::wstring file = dirIter->path();
                    sjejhh_pack_add_file(pPackContext, file.c_str());
                    dirIter++;
                }

                bool ret = sjejhh_pack_do_pack(pPackContext, nullptr, nullptr) == SJEJHH_PACK_OK;

                sjejhh_pack_close(pPackContext);

                return ret;
            }

            bool ReadGameConfig(GameConfig& out)
            {
                // unpack SJE.JHH archive
                auto pAppModel = AppModel::GetInstance();
                filesystem::path archivePath = pAppModel->GetGameExecutablePath().parent_path() / filesystem::path(L"config") / L"SJE.JHH";

                auto pEnv = Environment::GetInstance();
                filesystem::path unpackPath = filesystem::path(pEnv->GetAppTempDirectory().toStdWString()) / L"config";

                std::string internalFolderName;
                if (!UnpackSJEJHHArchive(archivePath, unpackPath, internalFolderName))
                {
                    return false;
                }

                filesystem::path configIniPath = unpackPath / L"config_jpn.ini";

                INIReader reader(configIniPath.string());

                out.moveSensitivity = reader.GetInteger("CHARACTER_A", "MoveDelay", 45);
                out.moveSpeed = reader.GetInteger("CHARACTER_A", "MoveInterval", 15);
                out.softDropSpeed = reader.GetInteger("CHARACTER_A", "SoftDropDelay", 10);

                out.lineClearDelay = reader.GetInteger("CHARACTER_A", "MinoGravity", 50);

                out.nextPiecesCount = reader.GetInteger("CHARACTER_A", "NextBlockCount", 6);

                return true;
            }
            bool WriteGameConfig(const GameConfig& config)
            {
                // unpack SJE.JHH archive
                auto pAppModel = AppModel::GetInstance();
                filesystem::path archivePath = pAppModel->GetGameExecutablePath().parent_path() / filesystem::path(L"config") / L"SJE.JHH";
                auto pEnv = Environment::GetInstance();
                filesystem::path unpackPath = filesystem::path(pEnv->GetAppTempDirectory().toStdWString()) / L"config";

                std::string internalFolderName;
                if (!UnpackSJEJHHArchive(archivePath, unpackPath, internalFolderName))
                {
                    return false;
                }

                filesystem::path configIniPath = unpackPath / L"config_jpn.ini";

                INIReader reader(configIniPath.string());

                {
                    std::string strMoveSensitivity = std::to_string(config.moveSensitivity);
                    std::string strMoveSpeed = std::to_string(config.moveSpeed);
                    std::string strMoveSoftDropSpeed = std::to_string(config.softDropSpeed);

                    reader.Set("CHARACTER_A", "MoveDelay", strMoveSensitivity);
                    reader.Set("CHARACTER_A", "MoveInterval", strMoveSpeed);
                    reader.Set("CHARACTER_A", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("CHARACTER_A", "SoftDropInterval", strMoveSoftDropSpeed);

                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveDelay", strMoveSensitivity);
                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveInterval", strMoveSpeed);
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveDelay", strMoveSensitivity);
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveInterval", strMoveSpeed);
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveDelay", strMoveSensitivity);
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveInterval", strMoveSpeed);
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveDelay", strMoveSensitivity);
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveInterval", strMoveSpeed);
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveDelay", strMoveSensitivity);
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveInterval", strMoveSpeed);

                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropInterval", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropInterval", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropInterval", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropInterval", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropDelay", strMoveSoftDropSpeed);
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropInterval", strMoveSoftDropSpeed);
                }

                {
                    std::string strlineClearDelay = std::to_string(config.lineClearDelay);

                    reader.Set("CHARACTER_A", "MinoGravity", strlineClearDelay);

                    reader.Set("UPGRADE_LINECLEARSPEED_LV1", "MinoGravity", strlineClearDelay);
                    reader.Set("UPGRADE_LINECLEARSPEED_LV2", "MinoGravity", strlineClearDelay);
                    reader.Set("UPGRADE_LINECLEARSPEED_LV3", "MinoGravity", strlineClearDelay);
                    reader.Set("UPGRADE_LINECLEARSPEED_LV4", "MinoGravity", strlineClearDelay);
                    reader.Set("UPGRADE_LINECLEARSPEED_LV5", "MinoGravity", strlineClearDelay);
                }

                {
                    std::string strNextPiecesCount = std::to_string(config.nextPiecesCount);

                    reader.Set("CHARACTER_A", "NextBlockCount", strNextPiecesCount);
                    reader.Set("CHARACTER_B", "NextBlockCount", strNextPiecesCount);
                    reader.Set("CHARACTER_C", "NextBlockCount", strNextPiecesCount);
                    reader.Set("CHARACTER_D", "NextBlockCount", strNextPiecesCount);


                    reader.Set("UPGRADE_NEXT_LV1", "NextBlockCount", strNextPiecesCount);
                    reader.Set("UPGRADE_NEXT_LV2", "NextBlockCount", strNextPiecesCount);
                    reader.Set("UPGRADE_NEXT_LV3", "NextBlockCount", strNextPiecesCount);
                    reader.Set("UPGRADE_NEXT_LV4", "NextBlockCount", strNextPiecesCount);
                    reader.Set("UPGRADE_NEXT_LV5", "NextBlockCount", strNextPiecesCount);
                }

                reader.WriteINIFile(configIniPath.string());

                // pack the archive again
                filesystem::path newArchivePath = archivePath;
                newArchivePath += "-";
                if (!PackSJEJHHArchive(unpackPath, newArchivePath, internalFolderName))
                {
                    return false;
                }

                filesystem::copy(newArchivePath, archivePath, filesystem::copy_options::overwrite_existing);
                filesystem::remove(newArchivePath);

                return true;
            }
            bool WriteTOPDefaultConfig()
            {
                // unpack SJE.JHH archive
                auto pAppModel = AppModel::GetInstance();
                filesystem::path archivePath = pAppModel->GetGameExecutablePath().parent_path() / filesystem::path(L"config") / L"SJE.JHH";
                auto pEnv = Environment::GetInstance();
                filesystem::path unpackPath = filesystem::path(pEnv->GetAppTempDirectory().toStdWString()) / L"config";

                std::string internalFolderName;
                if (!UnpackSJEJHHArchive(archivePath, unpackPath, internalFolderName))
                {
                    return false;
                }

                filesystem::path configIniPath = unpackPath / L"config_jpn.ini";

                INIReader reader(configIniPath.string());

                {
                    reader.Set("CHARACTER_A", "MoveDelay", "190");
                    reader.Set("CHARACTER_A", "MoveInterval", "95");
                    reader.Set("CHARACTER_A", "SoftDropDelay", "40");
                    reader.Set("CHARACTER_A", "SoftDropInterval", "40");
                    reader.Set("CHARACTER_A", "MinoGravity", "90");

                    reader.Set("DEFAULT_CHARACTER", "NextBlockCount", "6");
                    reader.Set("CHARACTER_A", "NextBlockCount", "1");
                    reader.Set("CHARACTER_B", "NextBlockCount", "6");
                    reader.Set("CHARACTER_C", "NextBlockCount", "3");
                    reader.Set("CHARACTER_D", "NextBlockCount", "6");

                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveDelay", "180");
                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveInterval", "50");
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveDelay", "165");
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveInterval", "30");
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveDelay", "130");
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveInterval", "30");
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveDelay", "130");
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveInterval", "15");
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveDelay", "100");
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveInterval", "15");

                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropDelay", "35");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropInterval", "35");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropDelay", "30");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropInterval", "30");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropDelay", "25");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropInterval", "25");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropDelay", "15");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropInterval", "15");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropDelay", "10");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropInterval", "10");

                    reader.Set("UPGRADE_LINECLEARSPEED_LV1", "MinoGravity", "80");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV2", "MinoGravity", "75");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV3", "MinoGravity", "70");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV4", "MinoGravity", "60");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV5", "MinoGravity", "50");

                    reader.Set("UPGRADE_NEXT_LV1", "NextBlockCount", "2");
                    reader.Set("UPGRADE_NEXT_LV2", "NextBlockCount", "3");
                    reader.Set("UPGRADE_NEXT_LV3", "NextBlockCount", "4");
                    reader.Set("UPGRADE_NEXT_LV4", "NextBlockCount", "5");
                    reader.Set("UPGRADE_NEXT_LV5", "NextBlockCount", "6");
                }

                reader.WriteINIFile(configIniPath.string());

                // pack the archive again
                filesystem::path newArchivePath = archivePath;
                newArchivePath += "-";
                if (!PackSJEJHHArchive(unpackPath, newArchivePath, internalFolderName))
                {
                    return false;
                }

                filesystem::copy(newArchivePath, archivePath, filesystem::copy_options::overwrite_existing);
                filesystem::remove(newArchivePath);

                return true;
            }
            bool WriteTOJDefaultConfig()
            {
                // unpack SJE.JHH archive
                auto pAppModel = AppModel::GetInstance();
                filesystem::path archivePath = pAppModel->GetGameExecutablePath().parent_path() / filesystem::path(L"config") / L"SJE.JHH";
                auto pEnv = Environment::GetInstance();
                filesystem::path unpackPath = filesystem::path(pEnv->GetAppTempDirectory().toStdWString()) / L"config";

                std::string internalFolderName;
                if (!UnpackSJEJHHArchive(archivePath, unpackPath, internalFolderName))
                {
                    return false;
                }

                filesystem::path configIniPath = unpackPath / L"config_jpn.ini";

                INIReader reader(configIniPath.string());

                {
                    reader.Set("CHARACTER_A", "MoveDelay", "190");
                    reader.Set("CHARACTER_A", "MoveInterval", "95");
                    reader.Set("CHARACTER_A", "SoftDropDelay", "40");
                    reader.Set("CHARACTER_A", "SoftDropInterval", "40");
                    reader.Set("CHARACTER_A", "MinoGravity", "90");

                    reader.Set("DEFAULT_CHARACTER", "NextBlockCount", "6");
                    reader.Set("CHARACTER_A", "NextBlockCount", "1");
                    reader.Set("CHARACTER_B", "NextBlockCount", "6");
                    reader.Set("CHARACTER_C", "NextBlockCount", "3");
                    reader.Set("CHARACTER_D", "NextBlockCount", "6");

                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveDelay", "180");
                    reader.Set("UPGRADE_LRSPEED_LV1", "MoveInterval", "90");
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveDelay", "160");
                    reader.Set("UPGRADE_LRSPEED_LV2", "MoveInterval", "80");
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveDelay", "140");
                    reader.Set("UPGRADE_LRSPEED_LV3", "MoveInterval", "70");
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveDelay", "110");
                    reader.Set("UPGRADE_LRSPEED_LV4", "MoveInterval", "55");
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveDelay", "80");
                    reader.Set("UPGRADE_LRSPEED_LV5", "MoveInterval", "40");

                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropDelay", "35");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV1", "SoftDropInterval", "35");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropDelay", "30");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV2", "SoftDropInterval", "30");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropDelay", "25");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV3", "SoftDropInterval", "25");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropDelay", "15");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV4", "SoftDropInterval", "15");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropDelay", "10");
                    reader.Set("UPGRADE_SOFTDROPSPEED_LV5", "SoftDropInterval", "10");

                    reader.Set("UPGRADE_LINECLEARSPEED_LV1", "MinoGravity", "80");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV2", "MinoGravity", "75");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV3", "MinoGravity", "70");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV4", "MinoGravity", "60");
                    reader.Set("UPGRADE_LINECLEARSPEED_LV5", "MinoGravity", "50");

                    reader.Set("UPGRADE_NEXT_LV1", "NextBlockCount", "2");
                    reader.Set("UPGRADE_NEXT_LV2", "NextBlockCount", "3");
                    reader.Set("UPGRADE_NEXT_LV3", "NextBlockCount", "4");
                    reader.Set("UPGRADE_NEXT_LV4", "NextBlockCount", "5");
                    reader.Set("UPGRADE_NEXT_LV5", "NextBlockCount", "6");
                }

                reader.WriteINIFile(configIniPath.string());

                // pack the archive again
                filesystem::path newArchivePath = archivePath;
                newArchivePath += "-";
                if (!PackSJEJHHArchive(unpackPath, newArchivePath, internalFolderName))
                {
                    return false;
                }

                filesystem::copy(newArchivePath, archivePath, filesystem::copy_options::overwrite_existing);
                filesystem::remove(newArchivePath);

                return true;
            }
}

    }
}
