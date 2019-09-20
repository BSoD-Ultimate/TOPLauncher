#include "stdafx.h"
#include "SettingsWidget.h"
#include "MainWindow.h"
#include "MainWidget.h"

#include "AppModel.h"
#include "LanguageItemModel.h"

namespace TOPLauncher
{
    Q_DECLARE_METATYPE(std::shared_ptr<ServerData>);

    class SettingDlgServerListModel : public QAbstractItemModel
    {
    public:
        SettingDlgServerListModel(SettingsWidget* pParent)
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
                    return QVariant::fromValue(std::shared_ptr<ServerData>());
                }
                else if (index.row() < pAppModel->GetServerData().size() + 1)
                {
                    auto& serverList = pAppModel->GetServerData();
                    auto serverData = serverList[dataIndex];

                    return QVariant::fromValue(std::shared_ptr<ServerData>(serverData));
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
        SettingsWidget* m_pParent;
        std::shared_ptr<AppModel> pAppModel;
    };



    SettingsWidget::SettingsWidget(TOPLauncherMainWindow* pMainWindow, QWidget* parent)
        : QWidget(parent)
        , m_pServerListModel(new SettingDlgServerListModel(this))
        , m_pLanguageItemModel(new LanguageItemModel(this))
        , m_pMainWindow(pMainWindow)
        , m_bDlgInit(false)
    {
        ui.setupUi(this);
        assert(m_pMainWindow);

        //ui.btnSetting->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        //ui.settingsScrollArea->setAttribute(Qt::WA_TransparentForMouseEvents, false);

        LoadSettingsFromModel();
        m_bDlgInit = true;
    }

    SettingsWidget::~SettingsWidget()
    {
    }

    void SettingsWidget::LoadSettingsFromModel()
    {
        LoadGeneralSettings();
        LoadServerListSettings();
        LoadGameControlSettings();
    }

    void SettingsWidget::LoadGeneralSettings()
    {
        auto pAppModel = AppModel::GetInstance();

        ui.comboBoxLanguage->setModel(m_pLanguageItemModel.get());
        ui.comboBoxLanguage->setCurrentIndex(util::GetLanguageIndex(AppModel::GetInstance()->GetDisplayLanguage()));
        ui.editGameExecutablePath->setText(QString::fromStdWString(pAppModel->GetGameExecutablePath()));
    }

    void SettingsWidget::LoadServerListSettings()
    {
        ui.serverList->setModel(m_pServerListModel.get());
        QObject::connect(ui.serverList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(on_serverList_selectionchanged(QItemSelection, QItemSelection)));
        ui.serverList->setCurrentIndex(m_pServerListModel->index(0, 0));
    }

    void SettingsWidget::LoadGameControlSettings()
    {
        auto pAppModel = AppModel::GetInstance();

        if (pAppModel->IsGameConfigAvailable())
        {
            int moveSensitivity = 0, moveSpeed = 0, softDropSpeed = 0;
            pAppModel->GetSensitivityValue(moveSensitivity, moveSpeed, softDropSpeed);

            int lineClearDelay = 0;
            pAppModel->GetLineClearDelayValue(lineClearDelay);

            ui.sliderMoveSensitivity->setValue(moveSensitivity);
            ui.sliderMoveSpeed->setValue(moveSpeed);
            ui.sliderSoftDropSpeed->setValue(softDropSpeed);
            ui.sliderLineClearDelay->setValue(lineClearDelay);

            ui.spinBoxMoveSensitivity->setValue(moveSensitivity);
            ui.spinBoxMoveSpeed->setValue(moveSpeed);
            ui.spinBoxSoftDropSpeed->setValue(softDropSpeed);
            ui.spinBoxLineClearDelay->setValue(lineClearDelay);
        }

    }

    bool SettingsWidget::ApplyGameControlSettings()
    {
        auto pAppModel = AppModel::GetInstance();

        if (pAppModel->IsGameConfigAvailable())
        {
            int moveSensitivity = ui.sliderMoveSensitivity->value();
            int moveSpeed = ui.sliderMoveSpeed->value();
            int softDropSpeed = ui.sliderSoftDropSpeed->value();

            int lineClearDelay = ui.sliderLineClearDelay->value();


            bool ret =
                pAppModel->SetSensitivityValue(moveSensitivity, moveSpeed, softDropSpeed) &&
                pAppModel->SetLineClearDelayValue(lineClearDelay);

            return ret;
        }
        else
        {
            return false;
        }

    }

    void SettingsWidget::changeEvent(QEvent* event)
    {
        QWidget::changeEvent(event);
        if (QEvent::LanguageChange == event->type())
        {
            ui.retranslateUi(this);
        }
    }

    void SettingsWidget::resizeEvent(QResizeEvent * e)
    {
        QWidget::resizeEvent(e);
        QRegion reg(frameGeometry());
        reg -= QRegion(geometry());
        reg += childrenRegion();
        setMask(reg);
    }

    void SettingsWidget::done()
    {
        auto pAppModel = AppModel::GetInstance();

        pAppModel->SaveAppConfig();
    }

    void SettingsWidget::on_btnSetting_clicked()
    {
    }

    void SettingsWidget::on_comboBoxLanguage_currentIndexChanged(int index)
    {
        auto langId = ui.comboBoxLanguage->currentData(Qt::UserRole).toString();
        auto pAppModel = AppModel::GetInstance();
        if (pAppModel->GetDisplayLanguage() != langId.toStdWString() && m_bDlgInit)
        {
            pAppModel->SetDisplayLanguage(langId.toStdWString());
        }
    }

    void SettingsWidget::on_btnUseSystemLang_clicked()
    {
        std::wstring langId = util::GetSystemPreferredLanguage();
        QString displayFormat = QObject::tr("Would you like to use your system language \"{}\" as the display language?");

        std::wstring langShow = util::GetLanguageShowString(langId);
        std::wstring tipString = util::wstring_format(displayFormat.toStdWString().c_str(), langShow);

        if (QMessageBox::question(this, QObject::tr("Question"), QString::fromStdWString(tipString), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            auto pAppModel = AppModel::GetInstance();
            if (pAppModel->SetDisplayLanguage(langId))
            {
                ui.comboBoxLanguage->setCurrentIndex(
                    util::GetLanguageIndex(pAppModel->GetDisplayLanguage()));
            }
        }
    }

    void SettingsWidget::on_btnBrowseGameExecutable_clicked()
    {
        QString gameExecutablePath = QFileDialog::getOpenFileName(this, QObject::tr("Find where game executable locates..."),
            QString(),
            QObject::tr("Game executable (tetris.exe)"), nullptr, 0);
        if (!gameExecutablePath.isEmpty())
        {
            auto pAppModel = AppModel::GetInstance();
            pAppModel->SetGameExecutablePath(gameExecutablePath.toStdWString());

            ui.editGameExecutablePath->setText(gameExecutablePath);

            // reload all settings related to the game client
            LoadGameControlSettings();
        }
    }

    void SettingsWidget::on_serverList_selectionchanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
        auto pAppModel = AppModel::GetInstance();
        auto serverData = ui.serverList->currentIndex().data(Qt::UserRole).value<std::shared_ptr<ServerData>>();
        

        if (!serverData)
        {
            // item "new server"
            ui.btnRemoveServerProfile->setEnabled(false);
            ui.editServerName->setText("");
            ui.editServerHost->setText("");
            ui.editRegisterURL->setText("");
        }
        else
        {
            // normal server profile
            ui.btnRemoveServerProfile->setEnabled(true);
            ui.editServerName->setText(QString::fromStdWString(serverData->serverName));
            ui.editServerHost->setText(QString::fromStdWString(serverData->serverAddress));
            ui.editRegisterURL->setText(QString::fromStdWString(serverData->registerURL));
        }

    }
    void SettingsWidget::on_btnSaveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();

        auto oldServerData = ui.serverList->currentIndex().data(Qt::UserRole).value<std::shared_ptr<ServerData>>();

        auto newServerData = std::make_shared<ServerData>();
        newServerData->serverName = ui.editServerName->text().toStdWString();
        newServerData->serverAddress = ui.editServerHost->text().toStdWString();
        newServerData->registerURL = ui.editRegisterURL->text().toStdWString();

        if (newServerData->serverName.empty())
        {
            QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("The field \"Server name\" should not empty."));
            return;
        }

        if (oldServerData)
        {
            // change server profile

            auto pExistingData = pAppModel->GetServerData(newServerData->serverName);

            // check name conflict
            if (!pExistingData || pExistingData == oldServerData)
            {
                pAppModel->ModifyServer(oldServerData->serverName, newServerData);
            }
            else
            {
                QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Server name must be unique."));
                return;
            }
        }
        else
        {
            // new server

            // check name conflict
            if (pAppModel->GetServerData(newServerData->serverName))
            {
                QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Server name must be unique."));
                return;
            }

            if (pAppModel->AddServer(newServerData))
            {
                m_pServerListModel->insertRow(m_pServerListModel->rowCount());
            }
        }

        if (m_pMainWindow)
        {
            //m_pMainWindow->on_serverSettingsChanged();
        }
        
    }

    void SettingsWidget::on_btnCancelServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();
        auto serverData = pAppModel->GetServerData();

        int index = ui.serverList->currentIndex().row();

        if (index == 0)
        {
            ui.editServerName->setText("");
            ui.editServerHost->setText("");
            ui.editRegisterURL->setText("");
        }
        else
        {
            std::shared_ptr<ServerData> pData = serverData[index - 1];
            assert(pData);
            ui.editServerName->setText(QString::fromStdWString(pData->serverName));
            ui.editServerHost->setText(QString::fromStdWString(pData->serverAddress));
            ui.editRegisterURL->setText(QString::fromStdWString(pData->registerURL));
        }
    }

    void SettingsWidget::on_btnRemoveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();
        std::wstring serverName = ui.serverList->currentIndex().data().toString().toStdWString();

        QString tipText = QObject::tr("Would you like to remove the server profile \"{}\" ?"
            " This will remove all saved users who login to this server.");

        std::wstring tipTextFormatted = util::wstring_format(tipText.toStdWString().c_str(), serverName);
        if (QMessageBox::question(this, QObject::tr("Question"), QString::fromStdWString(tipTextFormatted), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            if (pAppModel->RemoveServer(serverName))
            {
                m_pServerListModel->removeRow(m_pServerListModel->rowCount());
                ui.serverList->setCurrentIndex(m_pServerListModel->index(0, 0));
                if (m_pMainWindow)
                {
                    //m_pMainWindow->on_serverSettingsChanged();
                }
            }


        }


    }

    void SettingsWidget::on_btnApplyControlSettings_clicked()
    {
        if (QMessageBox::question(this, QObject::tr("Caution"), QObject::tr("Would you like to apply the new game control settings? "),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {

            bool ret = ApplyGameControlSettings();

            if (ret)
            {
                QMessageBox::information(this, QObject::tr("Info"), QObject::tr("Successfully applied new game control settings. "), QMessageBox::Ok);
            }
            else
            {
                QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Unable to apply new game control settings. "), QMessageBox::Ok);
            }

        }

    }

    void SettingsWidget::on_btnResetControlSettings_clicked()
    {
        LoadGameControlSettings();

    }

    void SettingsWidget::on_sliderMoveSensitivity_valueChanged(int value)
    {
        int minimum = ui.sliderMoveSensitivity->minimum();
        int maximum = ui.sliderMoveSensitivity->maximum();
        if (!((value == minimum && value > ui.spinBoxMoveSensitivity->value())
            || value == maximum && value < ui.spinBoxMoveSensitivity->value()))
        {
            ui.spinBoxMoveSensitivity->setValue(value);
        }
    }

    void SettingsWidget::on_sliderMoveSpeed_valueChanged(int value)
    {
        int minimum = ui.sliderMoveSpeed->minimum();
        int maximum = ui.sliderMoveSpeed->maximum();
        if (!((value == minimum && value > ui.spinBoxMoveSpeed->value())
            || value == maximum && value < ui.spinBoxMoveSpeed->value()))
        {
            ui.spinBoxMoveSpeed->setValue(value);
        }
    }

    void SettingsWidget::on_sliderSoftDropSpeed_valueChanged(int value)
    {
        int minimum = ui.sliderSoftDropSpeed->minimum();
        int maximum = ui.sliderSoftDropSpeed->maximum();
        if (!((value == minimum && value > ui.spinBoxSoftDropSpeed->value())
            || value == maximum && value < ui.spinBoxSoftDropSpeed->value()))
        {
            ui.spinBoxSoftDropSpeed->setValue(value);
        }
    }

    void SettingsWidget::on_sliderLineClearDelay_valueChanged(int value)
    {
        int minimum = ui.sliderLineClearDelay->minimum();
        int maximum = ui.sliderLineClearDelay->maximum();
        if (!((value == minimum && value > ui.spinBoxLineClearDelay->value())
            || value == maximum && value < ui.spinBoxLineClearDelay->value()))
        {
            ui.spinBoxLineClearDelay->setValue(value);
        }
    }


}


