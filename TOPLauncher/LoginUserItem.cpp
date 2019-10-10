#include "stdafx.h"
#include "LoginUserItem.h"
#include "MainWidget.h"

#include "dbUser.h"

#include <QMessageBox>

namespace TOPLauncher
{

    LoginUserItem::LoginUserItem(QWidget *parent, MainWidget& ownerWindow, const QString& username, const QString& serverName)
        : QWidget(parent)
        , m_ownerWindow(ownerWindow)
        , m_username(username)
        , m_serverName(serverName)
    {
        ui.setupUi(this);
    }

    LoginUserItem::~LoginUserItem()
    {
    }
    QString LoginUserItem::username() const
    {
        return m_username;
    }
    QString LoginUserItem::serverName() const
    {
        return m_serverName;
    }
    void LoginUserItem::on_btnRemoveUser_clicked()
    {
        QString tipText = tr("Would you like to remove the user \"%1\" who logins to server \"%2\"? ").arg(m_username, m_serverName);

        if (QMessageBox::question(parentWidget(), tr("Caution"), tipText, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            db::RemoveLoginUser(m_serverName, m_username);
            m_ownerWindow.ReloadServerData(m_serverName);
        }
    }
}

