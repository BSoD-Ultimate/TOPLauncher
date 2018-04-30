#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TOPLauncher.h"

namespace TOPLauncher
{
    namespace db
    {
        struct DBUserData;
        struct DBServerData;
    }

    class ServerListModel;
    class UserListModel;

    class TOPLauncherMainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        TOPLauncherMainWindow(QWidget *parent = Q_NULLPTR);
        ~TOPLauncherMainWindow();

        void LoadLastLoginUser(const std::wstring& serverName);
        void ReloadServerData(const std::wstring& serverName);
        void ReloadUserData(const std::wstring& username, const std::wstring& serverName);

    private:

        void InitData();

        void InsertLoginUserItem(QListWidget* pListWidget, const std::shared_ptr<db::DBUserData> pUserData);



    private slots:

        void on_btnExit_clicked();
        void on_btnLogin_clicked();
        void on_btnSettings_clicked();
        void on_btnLanguage_clicked();
        void on_btnRegister_clicked();
        void on_btnOpenForum_clicked();

        void on_comboServer_currentIndexChanged(int index);
        void on_comboUsername_currentIndexChanged(int index);
        void on_comboUsername_currentTextChanged(QString text);

    private:
        Ui::TOPLauncherClass ui;

        QListWidget* m_pComboUserPopupList;
        std::unique_ptr<ServerListModel> m_pServerListModel;
    };
}

