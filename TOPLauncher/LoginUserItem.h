/* 
 * item widget used in main window's user-name combobox drop-down list
*/
#pragma once

#include <QtWidgets/QWidget>
#include "ui_LoginUserItem.h"

namespace TOPLauncher
{
    class MainWidget;

    class LoginUserItem : public QWidget
    {
        Q_OBJECT

    public:
        LoginUserItem(QWidget *parent, MainWidget& pOwnerWindow, const std::wstring& username, const std::wstring& serverName);
        ~LoginUserItem();

        std::wstring username() const;
        std::wstring serverName() const;

    private slots:
        void on_btnRemoveUser_clicked();

    private:
        Ui::LoginUserItem ui;

        MainWidget& m_ownerWindow;

        std::wstring m_username;
        std::wstring m_serverName;
    };

}

