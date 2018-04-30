#pragma once

#include <QtWidgets/QWidget>
#include "ui_LoginUserItem.h"

namespace TOPLauncher
{
    class TOPLauncherMainWindow;

    class LoginUserItem : public QWidget
    {
        Q_OBJECT

    public:
        LoginUserItem(QWidget *parent, TOPLauncherMainWindow& pOwnerWindow, const std::wstring& username, const std::wstring& serverName);
        ~LoginUserItem();

        std::wstring username() const;
        std::wstring serverName() const;

    private slots:
        void on_btnRemoveUser_clicked();

    private:
        Ui::LoginUserItem ui;

        TOPLauncherMainWindow& m_ownerWindow;

        std::wstring m_username;
        std::wstring m_serverName;
    };

}

