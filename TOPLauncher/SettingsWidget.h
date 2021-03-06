/*
 * Setting dialog
*/
#pragma once

#include <QtWidgets/QDialog>
#include <QButtonGroup>
#include "ui_SettingsWidget.h"

namespace TOPLauncher
{
    class TOPLauncherMainWindow;

    class LanguageItemModel;
    class SettingDlgServerListModel;

    class SettingsWidget : public QWidget
    {
        Q_OBJECT
    public:
        SettingsWidget(TOPLauncherMainWindow* pMainWindow, QWidget *parent = Q_NULLPTR);
        ~SettingsWidget();

    public:
        int GetScrollAreaLocationX() const;
        void SetSettingButtonState(bool showClosePrompt);
        void EnableGameSettings(bool enable);

    signals:
        void ServerSettingsChanged();
        void SettingButtonToggled(bool showClosePrompt);

    private:
		void InitUI();

        void LoadSettingsFromModel();

        void LoadGeneralSettings();
        void LoadServerListSettings();
        void LoadGameControlSettings();

        bool ApplyGameControlSettings();
        
        // QWidget
        void changeEvent(QEvent* e) override;
        void resizeEvent(QResizeEvent* e) override;
        // QDialog
        void done();

    private slots :

    void on_btnSetting_toggled(bool checked);

    void on_comboBoxLanguage_currentIndexChanged(int index);
    void on_btnUseSystemLang_clicked();
    void on_btnBrowseGameExecutable_clicked();

    void on_serverList_selectionchanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_btnSaveServerProfile_clicked();
    void on_btnCancelServerProfile_clicked();
    void on_btnRemoveServerProfile_clicked();

    void on_btnApplyControlSettings_clicked();
    void on_btnResetControlSettings_clicked();
    void on_btnResetTOPDefault_clicked();
    void on_btnResetTOJDefault_clicked();

    void on_btnUnpackArchive_clicked();
    void on_btnPackArchive_clicked();
    void on_btnOpenExtractedFolder_clicked();

    // spinbox changed
    void on_sliderMoveSensitivity_valueChanged(int value);
    void on_sliderMoveSpeed_valueChanged(int value);
    void on_sliderSoftDropSpeed_valueChanged(int value);
    void on_sliderLineClearDelay_valueChanged(int value);

    

    private:
        Ui::SettingsWidget ui;
		QButtonGroup m_nextPiecesGroup;

        TOPLauncherMainWindow* m_pMainWindow;

        std::unique_ptr<SettingDlgServerListModel> m_pServerListModel;
        std::unique_ptr<LanguageItemModel> m_pLanguageItemModel;

        bool m_bDlgInit;
    };

}

