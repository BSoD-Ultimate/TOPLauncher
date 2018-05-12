#include "stdafx.h"
#include "DlgSettings.h"
#include "TOPLauncherMainWindow.h"


#include "dbServer.h"

#include "AppModel.h"

namespace TOPLauncher
{
    Q_DECLARE_METATYPE(std::shared_ptr<db::DBServerData>);

    class SettingDlgLanguageModel : public QAbstractItemModel
    {
    public:
        SettingDlgLanguageModel(DlgSettings* pParent)
            : m_pParent(pParent)
        {

        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {
            return createIndex(row, column, nullptr);
        }
        QModelIndex parent(const QModelIndex &child) const override
        {
            return QModelIndex();
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return 0;
        }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return 1;
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            return QVariant();
        }


    private:
        DlgSettings* m_pParent;
    };

    class SettingDlgServerListModel : public QAbstractItemModel
    {
    public:
        SettingDlgServerListModel(DlgSettings* pParent)
            : m_pParent(pParent)
            , pAppModel(AppModel::GetInstance())
        {

        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {
            return createIndex(row, column, nullptr);
        }
        QModelIndex parent(const QModelIndex &child) const override
        {
            return QModelIndex();
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            auto serverList = pAppModel->GetServerData();
            return serverList.size() + 1;
        }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return 1;
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            if (role == Qt::DisplayRole)
            {
                if (index.row() == 0)
                {
                    return QVariant(QObject::tr("New server..."));
                }
                else if(index.row() < pAppModel->GetServerData().size() + 1)
                {
                    int dataIndex = index.row() - 1;

                    auto& serverList = pAppModel->GetServerData();
                    auto serverData = serverList[dataIndex];
                    assert(serverData);
                    if (!serverData)
                    {
                        return QVariant();
                    }
                    return QVariant(QString::fromStdWString(serverList[dataIndex]->serverName));
                }
                else
                {
                    return QVariant();
                }
            }
            else if (role & Qt::UserRole)
            {
                int dataIndex = index.row() - 1;

                if (index.row() == 0)
                {
                    return QVariant::fromValue(std::shared_ptr<db::DBServerData>());
                }
                else if (index.row() < pAppModel->GetServerData().size() + 1)
                {
                    auto& serverList = pAppModel->GetServerData();
                    auto serverData = serverList[dataIndex];

                    return QVariant::fromValue(std::shared_ptr<db::DBServerData>(serverData));
                }
                else
                {
                    return QVariant();
                }
            }
            return QVariant();
        }

        bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
        {
            beginInsertRows(parent, row, row + count);

            endInsertRows();
            return true;
        }

        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
        {
            beginRemoveRows(parent, row, row + count);
            endRemoveRows();
            return true;
        }


    private:
        DlgSettings* m_pParent;
        std::shared_ptr<AppModel> pAppModel;
    };



    DlgSettings::DlgSettings(QWidget* parent)
        : QDialog(parent)
        , m_pLanguageComboboxModel(new SettingDlgLanguageModel(this))
        , m_pServerListModel(new SettingDlgServerListModel(this))
        , m_pMainWindow(dynamic_cast<TOPLauncherMainWindow*>(parent))
    {
        ui.setupUi(this);
        ui.serverList->setModel(m_pServerListModel.get());
        ui.serverList->setCurrentIndex(m_pServerListModel->index(0, 0));

        assert(m_pMainWindow);

        LoadSettingsFromModel();
    }

    DlgSettings::~DlgSettings()
    {
    }

    void DlgSettings::LoadSettingsFromModel()
    {
        auto pAppModel = AppModel::GetInstance();
        ui.editGameExecutablePath->setText(QString::fromStdWString(pAppModel->GetGameExecutablePath()));

        int moveSensitivity = 0, moveSpeed = 0, softDropSpeed = 0;
        pAppModel->GetSensitivityValue(moveSensitivity, moveSpeed, softDropSpeed);

        ui.sliderMoveSensitivity->setValue(moveSensitivity);
        ui.sliderMoveSpeed->setValue(moveSpeed);
        ui.sliderSoftDropSpeed->setValue(softDropSpeed);
    }

    bool DlgSettings::CheckReservedServerData(const std::wstring& serverName)
    {
        auto pAppModel = AppModel::GetInstance();
        if (pAppModel->IsReservedServer(serverName))
        {
            QMessageBox::critical(this, QObject::tr("Error"),
                QObject::tr("Current server profile is reserved by application, which cannot be modified, removed or conflict with another server profile."));
            return true;
        }
        return false;
    }

    void DlgSettings::on_comboBoxLanguage_currentIndexChanged(int index)
    {
    }

    void DlgSettings::on_btnBrowseGameExecutable_clicked()
    {
        QString gameExecutablePath = QFileDialog::getOpenFileName(this, QObject::tr("Find where game executable locates..."),
            QString(),
            QObject::tr("Game executable (tetris.exe)"), nullptr, 0);
        if (!gameExecutablePath.isEmpty())
        {
            auto pAppModel = AppModel::GetInstance();
            pAppModel->SetGameExecutablePath(gameExecutablePath.toStdWString());

            ui.editGameExecutablePath->setText(gameExecutablePath);
        }
    }

    void DlgSettings::on_serverList_clicked(const QModelIndex &index)
    {
        auto pAppModel = AppModel::GetInstance();
        auto serverData = ui.serverList->currentIndex().data(Qt::UserRole).value<std::shared_ptr<db::DBServerData>>();
        
        // create new server
        if (!serverData)
        {
            ui.btnRemoveServerProfile->setEnabled(false);
            ui.editServerName->setText("");
            ui.editServerHost->setText("");
        }
        else
        {
            ui.btnRemoveServerProfile->setEnabled(true);
            ui.editServerName->setText(QString::fromStdWString(serverData->serverName));
            ui.editServerHost->setText(QString::fromStdWString(serverData->serverAddress));
        }

    }
    void DlgSettings::on_btnSaveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();

        db::DBServerData newServerData;
        newServerData.serverName = ui.editServerName->text().toStdWString();
        newServerData.serverAddress = ui.editServerHost->text().toStdWString();

        if (CheckReservedServerData(newServerData.serverName))
        {
            return;
        }

        if (newServerData.serverName.empty())
        {
            QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("The field \"Server name\" should not empty."));
            return;
        }

        auto oldServerData = ui.serverList->currentIndex().data(Qt::UserRole).value<std::shared_ptr<db::DBServerData>>();

        if (pAppModel->GetServerData(newServerData.serverName) != oldServerData)
        {
            QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Server name must be unique."));
            return;
        }



        if (oldServerData)
        {
            pAppModel->ModifyServer(oldServerData->serverName, newServerData);
        }
        else
        {
            if (pAppModel->AddServer(newServerData))
            {
                m_pServerListModel->insertRow(m_pServerListModel->rowCount());
            }
        }

        if (m_pMainWindow)
        {
            m_pMainWindow->on_serverSettingsChanged();
        }
        
    }
    void DlgSettings::on_btnCancelServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();
        auto serverData = pAppModel->GetServerData();

        int index = ui.serverList->currentIndex().row();

        if (index == 0)
        {
            ui.editServerName->setText("");
            ui.editServerHost->setText("");
        }
        else
        {
            std::shared_ptr<db::DBServerData> pData = serverData[index - 1];
            assert(pData);
            ui.editServerName->setText(QString::fromStdWString(pData->serverName));
            ui.editServerHost->setText(QString::fromStdWString(pData->serverAddress));
        }
    }
    void DlgSettings::on_btnRemoveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();
        std::wstring serverName = ui.serverList->currentIndex().data().toString().toStdWString();

        if (CheckReservedServerData(serverName))
        {
            return;
        }

        QString tipText = QObject::tr("Would you like to remove the server profile \"{}\" ?"
            " This will remove all saved users who login to this server.");

        std::wstring tipTextFormatted = util::wstring_format(tipText.toStdWString().c_str(), serverName);
        if (QMessageBox::question(this, QObject::tr("Question"), QString::fromStdWString(tipTextFormatted), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            if (pAppModel->RemoveServer(serverName))
            {
                m_pServerListModel->removeRow(m_pServerListModel->rowCount());
                if (m_pMainWindow)
                {
                    m_pMainWindow->on_serverSettingsChanged();
                }
            }


        }


    }

    void DlgSettings::on_btnApplySensitivitySettings_clicked()
    {
        if (QMessageBox::question(this, QObject::tr("Caution"), QObject::tr("Would you like to apply the new keypress sensitivity settings? "),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            auto pAppModel = AppModel::GetInstance();

            int moveSensitivity = ui.sliderMoveSensitivity->value();
            int moveSpeed = ui.sliderMoveSpeed->value();
            int softDropSpeed = ui.sliderSoftDropSpeed->value();

            bool ret = pAppModel->SetSensitivityValue(moveSensitivity, moveSpeed, softDropSpeed);

            if (ret)
            {
                QMessageBox::information(this, QObject::tr("Info"), QObject::tr("Successfully applied new keypress sensitivity settings. "), QMessageBox::Ok);
            }
            else
            {
                QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Unable to apply new sensitivity settings. "), QMessageBox::Ok);
            }

        }

    }
    void DlgSettings::on_btnResetSensitivitySettings_clicked()
    {
        auto pAppModel = AppModel::GetInstance();

        int moveSensitivity = 0, moveSpeed = 0, softDropSpeed = 0;
        pAppModel->GetSensitivityValue(moveSensitivity, moveSpeed, softDropSpeed);

        ui.sliderMoveSensitivity->setValue(moveSensitivity);
        ui.sliderMoveSpeed->setValue(moveSpeed);
        ui.sliderSoftDropSpeed->setValue(softDropSpeed);
    }


}


