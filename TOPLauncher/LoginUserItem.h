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
        LoginUserItem(QWidget *parent, MainWidget& pOwnerWindow, const QString& username, const QString& serverName);
        ~LoginUserItem();

        QString username() const;
        QString serverName() const;

    private slots:
        void on_btnRemoveUser_clicked();

    private:
        Ui::LoginUserItem ui;

        MainWidget& m_ownerWindow;

        QString m_username;
        QString m_serverName;
    };

}

