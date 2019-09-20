/*
 * Main Widget
*/
#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include "ui_MainWidget.h"

namespace TOPLauncher
{
    namespace db
    {
        struct DBUserData;
    }

    class ServerDropListModel;
    class UserListModel;

    class MainWidget : public QWidget
    {
        Q_OBJECT

    public:
        MainWidget(QWidget *parent = Q_NULLPTR);
        ~MainWidget();

        void LoadLastLoginUser(const std::wstring& serverName);
        void ReloadServerData(const std::wstring& serverName);
        void ReloadUserData(const std::wstring& username, const std::wstring& serverName);

    private:

        void InitData();
        void InsertLoginUserItem(QListWidget* pListWidget, const std::shared_ptr<db::DBUserData> pUserData);
        void changeEvent(QEvent* event) override;

    public slots:
        void on_serverSettingsChanged();

    private slots:

        void on_btnExit_clicked();
        void on_btnLogin_clicked();
        void on_btnLanguage_clicked();
        void on_btnRegister_clicked();
        void on_btnOpenForum_clicked();

        void on_comboServer_currentIndexChanged(int index);
        void on_comboUsername_currentIndexChanged(int index);
        void on_comboUsername_currentTextChanged(QString text);

    private:
        Ui::MainWidget ui;

        QListWidget* m_pComboUserPopupList;
        std::unique_ptr<ServerDropListModel> m_pServerListModel;
    };
}

