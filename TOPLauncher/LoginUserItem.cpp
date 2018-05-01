#include "stdafx.h"
#include "LoginUserItem.h"

#include "TOPLauncherMainWindow.h"

#include "dbUser.h"

namespace TOPLauncher
{

    LoginUserItem::LoginUserItem(QWidget *parent, TOPLauncherMainWindow& ownerWindow, const std::wstring& username, const std::wstring& serverName)
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
    std::wstring LoginUserItem::username() const
    {
        return m_username;
    }
    std::wstring LoginUserItem::serverName() const
    {
        return m_serverName;
    }
    void LoginUserItem::on_btnRemoveUser_clicked()
    {
        QString tipText = QObject::tr("Would you like to remove the user \"{}\" who logins to server \"{}\"? ");

        std::wstring tipTextFormatted = util::wstring_format(tipText.toStdWString().c_str(), m_username, m_serverName);

        if (QMessageBox::question(parentWidget(), QObject::tr("Caution"), QString::fromStdWString(tipTextFormatted), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            db::RemoveLoginUser(m_serverName, m_username);
            m_ownerWindow.ReloadServerData(m_serverName);
        }
    }
}

