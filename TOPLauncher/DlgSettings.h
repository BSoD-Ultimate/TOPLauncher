/*
 * Setting dialog
*/
#pragma once

#include <QtWidgets/QDialog>
#include "ui_DlgSettings.h"

namespace TOPLauncher
{
    class TOPLauncherMainWindow;

    class SettingDlgLanguageModel;
    class SettingDlgServerListModel;

    class DlgSettings : public QDialog
    {
        Q_OBJECT


    public:
        DlgSettings(QWidget *parent = Q_NULLPTR);
        ~DlgSettings();

    public:


    signals:
        void ServerSettingsChanged();

    private:
        void LoadSettingsFromModel();
        bool CheckReservedServerData(const std::wstring& serverName);


    private slots :

    void on_comboBoxLanguage_currentIndexChanged(int index);
    void on_btnBrowseGameExecutable_clicked();

    void on_serverList_clicked(const QModelIndex &index);
    void on_btnSaveServerProfile_clicked();
    void on_btnCancelServerProfile_clicked();
    void on_btnRemoveServerProfile_clicked();

    void on_btnApplySensitivitySettings_clicked();
    void on_btnResetSensitivitySettings_clicked();


    private:
        Ui::DlgSettings ui;

        TOPLauncherMainWindow* m_pMainWindow;

        std::unique_ptr<SettingDlgLanguageModel> m_pLanguageComboboxModel;
        std::unique_ptr<SettingDlgServerListModel> m_pServerListModel;
    };

}

