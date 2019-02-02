/*
 * Setting dialog
*/
#pragma once

#include <QtWidgets/QDialog>
#include "ui_DlgSettings.h"

namespace TOPLauncher
{
    class TOPLauncherMainWindow;

    class LanguageItemModel;
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

        void LoadGeneralSettings();
        void LoadServerListSettings();
        void LoadGameControlSettings();

        bool ApplyGameControlSettings();
        
        // QWidget
        void changeEvent(QEvent* event) override;

        // QDialog
        void done(int retCode) override;

    private slots :

    void on_comboBoxLanguage_currentIndexChanged(int index);
    void on_btnUseSystemLang_clicked();
    void on_btnBrowseGameExecutable_clicked();

    void on_serverList_selectionchanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_btnSaveServerProfile_clicked();
    void on_btnCancelServerProfile_clicked();
    void on_btnRemoveServerProfile_clicked();

    void on_btnApplyControlSettings_clicked();
    void on_btnResetControlSettings_clicked();


    private:
        Ui::DlgSettings ui;

        TOPLauncherMainWindow* m_pMainWindow;

        std::unique_ptr<SettingDlgServerListModel> m_pServerListModel;
        std::unique_ptr<LanguageItemModel> m_pLanguageItemModel;

        bool m_bDlgInit;
    };

}

