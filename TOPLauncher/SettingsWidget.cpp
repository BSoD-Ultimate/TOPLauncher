#include "stdafx.h"
#include "SettingsWidget.h"
#include "MainWindow.h"
#include "MainWidget.h"
#include "DlgSJEJHHUnpack.h"
#include "DlgSJEJHHPack.h"


#include "AppModel.h"
#include "LanguageModel.h"
#include "LanguageItemModel.h"

#include <QMessageBox>
#include <QFileDialog>

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

        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
        {
            return createIndex(row, column, nullptr);
        }
        QModelIndex parent(const QModelIndex& child) const override
        {
            return QModelIndex();
        }

        int rowCount(const QModelIndex& parent = QModelIndex()) const override
        {
            auto serverList = pAppModel->GetServerData();
            return serverList.size() + 1;
        }
        int columnCount(const QModelIndex& parent = QModelIndex()) const override
        {
            return 1;
        }

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
        {
            if (role == Qt::DisplayRole)
            {
                if (index.row() == 0)
                {
                    return QVariant(QObject::tr("New server profile..."));
                }
                else if (index.row() < pAppModel->GetServerData().size() + 1)
                {
                    int dataIndex = index.row() - 1;

                    auto& serverList = pAppModel->GetServerData();
                    auto serverData = serverList[dataIndex];
                    assert(serverData);
                    if (!serverData)
                    {
                        return QVariant();
                    }
                    return QVariant(serverList[dataIndex]->serverName);
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

        bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
        {
            beginInsertRows(parent, row, row + count);

            endInsertRows();
            return true;
        }

        bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
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

        InitUI();

        EnableGameSettings(false);

        LoadSettingsFromModel();
        m_bDlgInit = true;
    }

    SettingsWidget::~SettingsWidget()
    {
    }

    int SettingsWidget::GetScrollAreaLocationX() const
    {
        auto btnSettingGeometry = ui.btnSetting->minimumWidth();
        return btnSettingGeometry;
    }

    void SettingsWidget::SetSettingButtonState(bool showClosePrompt)
    {
        ui.btnSetting->setChecked(showClosePrompt);
    }

    void SettingsWidget::EnableGameSettings(bool enable)
    {
        ui.groupServerSettings->setEnabled(enable);
        ui.groupControlSettings->setEnabled(enable);
    }

    void SettingsWidget::InitUI()
    {
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount1);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount1, 1);
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount2);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount2, 2);
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount3);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount3, 3);
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount4);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount4, 4);
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount5);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount5, 5);
        m_nextPiecesGroup.addButton(ui.radioNextPieceCount6);
        m_nextPiecesGroup.setId(ui.radioNextPieceCount6, 6);
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
        auto pLangModel = LanguageModel::GetInstance();

        auto displayLang = pAppModel->GetDisplayLanguage();
        int findIndex = -1;
        pLangModel->FindTranslator(displayLang, nullptr, &findIndex);

        ui.comboBoxLanguage->setModel(m_pLanguageItemModel.get());
        ui.comboBoxLanguage->setCurrentIndex(findIndex);
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
            EnableGameSettings(true);

            const auto& gameConfig = pAppModel->GetGameConfig();

            ui.sliderMoveSensitivity->setValue(gameConfig.moveSensitivity);
            ui.sliderMoveSpeed->setValue(gameConfig.moveSpeed);
            ui.sliderSoftDropSpeed->setValue(gameConfig.softDropSpeed);
            ui.sliderLineClearDelay->setValue(gameConfig.lineClearDelay);

            ui.spinBoxMoveSensitivity->setValue(gameConfig.moveSensitivity);
            ui.spinBoxMoveSpeed->setValue(gameConfig.moveSpeed);
            ui.spinBoxSoftDropSpeed->setValue(gameConfig.softDropSpeed);
            ui.spinBoxLineClearDelay->setValue(gameConfig.lineClearDelay);

            switch (gameConfig.nextPiecesCount)
            {
            case 1:
                ui.radioNextPieceCount1->setChecked(true);
                break;
            case 2:
                ui.radioNextPieceCount2->setChecked(true);
                break;
            case 3:
                ui.radioNextPieceCount3->setChecked(true);
                break;
            case 4:
                ui.radioNextPieceCount4->setChecked(true);
                break;
            case 5:
                ui.radioNextPieceCount5->setChecked(true);
                break;
            case 6:
                ui.radioNextPieceCount6->setChecked(true);
                break;
            default:
                break;
            }
        }
        else
        {
            EnableGameSettings(false);
        }

    }

    bool SettingsWidget::ApplyGameControlSettings()
    {
        auto pAppModel = AppModel::GetInstance();

        if (pAppModel->IsGameConfigAvailable())
        {
            auto newGameConfig = pAppModel->GetGameConfig();

            newGameConfig.moveSensitivity = ui.spinBoxMoveSensitivity->value();
            newGameConfig.moveSpeed = ui.spinBoxMoveSpeed->value();
            newGameConfig.softDropSpeed = ui.spinBoxSoftDropSpeed->value();

            newGameConfig.lineClearDelay = ui.spinBoxLineClearDelay->value();

            
            int checkedId = m_nextPiecesGroup.checkedId();
            newGameConfig.nextPiecesCount = checkedId;

            bool ret = pAppModel->ApplyGameConfig(newGameConfig);

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

            auto pAppModel = AppModel::GetInstance();
            auto pLangModel = LanguageModel::GetInstance();
            auto displayLang = pAppModel->GetDisplayLanguage();
            int findIndex = -1;
            pLangModel->FindTranslator(displayLang, nullptr, &findIndex);
            ui.comboBoxLanguage->setCurrentIndex(findIndex);
        }
    }

    void SettingsWidget::resizeEvent(QResizeEvent* e)
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

        pAppModel->SaveAppConfigToFile();
    }

    void SettingsWidget::on_btnSetting_toggled(bool checked)
    {
        emit SettingButtonToggled(checked);
    }

    void SettingsWidget::on_comboBoxLanguage_currentIndexChanged(int index)
    {
        auto langId = ui.comboBoxLanguage->currentData(Qt::UserRole).toString();
        auto pAppModel = AppModel::GetInstance();
        if (pAppModel->GetDisplayLanguage() != langId && m_bDlgInit)
        {
            pAppModel->SetDisplayLanguage(langId);
        }
    }

    void SettingsWidget::on_btnUseSystemLang_clicked()
    {
        QString langId = util::GetSystemLanguageName();
        auto pLangModel = LanguageModel::GetInstance();

        UITranslator* pTranslator = nullptr;
        int langIndex = 0;
        bool translatorFound = pLangModel->FindTranslator(langId, &pTranslator, &langIndex);

        if (translatorFound)
        {
            assert(pTranslator);

            QString langShow = pTranslator->LangShowName();
            QString displayFormat = tr("Would you like to use your system language \"%1\" as the display language?");
            QString promptText = displayFormat.arg(langShow);
            if (QMessageBox::question(this, tr("Question"), promptText, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            {
                auto pAppModel = AppModel::GetInstance();
                if (pAppModel->SetDisplayLanguage(langId))
                {
                    ui.comboBoxLanguage->setCurrentIndex(langIndex);
                }
            }
        }
        else
        {
            QString displayFormat = tr("Could not find translations for your system language \"%1\".");
            QString promptText = displayFormat.arg(langId);
            QMessageBox::critical(this, tr("Error"), promptText);
        }

    }

    void SettingsWidget::on_btnBrowseGameExecutable_clicked()
    {
        QString gameExecutablePath = QFileDialog::getOpenFileName(this, tr("Find where the game executable locates..."),
            QString(),
            tr("Game executable (tetris.exe);;Any executable (*.exe)"), nullptr, 0);
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
            ui.editServerName->setText(serverData->serverName);
            ui.editServerHost->setText(serverData->serverAddress);
            ui.editRegisterURL->setText(serverData->registerURL);
        }

    }
    void SettingsWidget::on_btnSaveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();

        auto oldServerData = ui.serverList->currentIndex().data(Qt::UserRole).value<std::shared_ptr<ServerData>>();

        auto newServerData = std::make_shared<ServerData>();
        newServerData->serverName = ui.editServerName->text();
        newServerData->serverAddress = ui.editServerHost->text();
        newServerData->registerURL = ui.editRegisterURL->text();

        if (newServerData->serverName.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("The field \"Server name\" should not empty."));
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
                QMessageBox::critical(this, tr("Error"), tr("Server name must be unique."));
                return;
            }
        }
        else
        {
            // new server

            // check name conflict
            if (pAppModel->GetServerData(newServerData->serverName))
            {
                QMessageBox::critical(this, tr("Error"), tr("Server name must be unique."));
                return;
            }

            if (pAppModel->AddServer(newServerData))
            {
                m_pServerListModel->insertRow(m_pServerListModel->rowCount());
            }
        }

        if (m_pMainWindow)
        {
            m_pMainWindow->GetMainWidget()->on_serverSettingsChanged();
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
            ui.editServerName->setText(pData->serverName);
            ui.editServerHost->setText(pData->serverAddress);
            ui.editRegisterURL->setText(pData->registerURL);
        }
    }

    void SettingsWidget::on_btnRemoveServerProfile_clicked()
    {
        auto pAppModel = AppModel::GetInstance();
        QString serverName = ui.serverList->currentIndex().data().toString();

        QString promptText = tr("Would you like to remove the server profile \"%1\" ?"
            " This will remove all saved users who login to this server.").arg(serverName);

        if (QMessageBox::question(this, tr("Question"), promptText, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            if (pAppModel->RemoveServer(serverName))
            {
                m_pServerListModel->removeRow(m_pServerListModel->rowCount());
                ui.serverList->setCurrentIndex(m_pServerListModel->index(0, 0));
                if (m_pMainWindow)
                {
                    m_pMainWindow->GetMainWidget()->on_serverSettingsChanged();
                }
            }
        }

    }

    void SettingsWidget::on_btnApplyControlSettings_clicked()
    {
        if (QMessageBox::question(this, tr("Caution"), tr("Would you like to apply the new game control settings? "),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {

            bool ret = ApplyGameControlSettings();

            if (ret)
            {
                QMessageBox::information(this, tr("Info"), tr("Successfully applied new game control settings. "), QMessageBox::Ok);
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), tr("Unable to apply new game control settings. "), QMessageBox::Ok);
            }

        }

    }

    void SettingsWidget::on_btnResetControlSettings_clicked()
    {
        LoadGameControlSettings();
    }

    void SettingsWidget::on_btnResetTOPDefault_clicked()
    {
        if (QMessageBox::question(this, tr("Caution"), tr("Reset the game control settings to TOP's shop default?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            auto pAppModel = AppModel::GetInstance();

            bool ret = pAppModel->ApplyGameConfigTOPDefault();

            if (ret)
            {
                QMessageBox::information(this, tr("Info"), tr("Successfully applied new game control settings. "), QMessageBox::Ok);
                LoadGameControlSettings();
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), tr("Unable to apply new game control settings. "), QMessageBox::Ok);
            }
        }
    }

    void SettingsWidget::on_btnResetTOJDefault_clicked()
    {
        if (QMessageBox::question(this, tr("Caution"), tr("Reset the game control settings to TOJ's shop default?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            auto pAppModel = AppModel::GetInstance();

            bool ret = pAppModel->ApplyGameConfigTOJDefualt();

            if (ret)
            {
                QMessageBox::information(this, tr("Info"), tr("Successfully applied new game control settings. "), QMessageBox::Ok);
                LoadGameControlSettings();
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), tr("Unable to apply new game control settings. "), QMessageBox::Ok);
            }
        }
    }

    void SettingsWidget::on_btnUnpackArchive_clicked()
    {
        DlgSJEJHHUnpack unpackDlg(this);

        connect(&unpackDlg, &DlgSJEJHHUnpack::ArchiveUnpackFinished,
            [this](QString archivePath, QString extractPath, QString internalFolderName) {
                ui.editArchivePath->setText(archivePath);
                ui.editArchiveFolderName->setText(internalFolderName);
                ui.editExtractPath->setText(extractPath);
            });

        unpackDlg.exec();
    }

    void SettingsWidget::on_btnPackArchive_clicked()
    {
        DlgSJEJHHPack packDlg(this);

        packDlg.setPackFolder(ui.editExtractPath->text());
        packDlg.setInternalFolderName(ui.editArchiveFolderName->text());
        packDlg.setArchiveFilePath(ui.editArchivePath->text());

        packDlg.exec();
    }

    void SettingsWidget::on_btnOpenExtractedFolder_clicked()
    {
        QString extractPath = ui.editExtractPath->text();

        extractPath.replace(QChar('/'), QChar('\\'));
        std::wstring cmd = L"\"" + extractPath.toStdWString() + L"\"";
        ShellExecuteW(NULL, L"open", L"explorer.exe", cmd.c_str(), NULL, SW_SHOW);
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


