#include "stdafx.h"
#include "TOPLauncherMainWindow.h"
#include "LoginUserItem.h"

#include "DlgLanguage.h"
#include "DlgSettings.h"

#include "AppModel.h"

#include "dbUser.h"
#include "dbServer.h"



namespace TOPLauncher
{
    Q_DECLARE_METATYPE(std::weak_ptr<db::DBServerData>);
    Q_DECLARE_METATYPE(std::shared_ptr<db::DBUserData>);

    class ServerDropListModel : public QAbstractItemModel
    {
    public:
        ServerDropListModel(TOPLauncherMainWindow& parent)
            : m_parent(parent)
        {
            auto pAppModel = AppModel::GetInstance();
            auto& serverList = pAppModel->GetServerData();

            for (const std::shared_ptr<db::DBServerData>& serverData : serverList)
            {
                assert(serverData);
                if (!serverData) continue;
                m_serverList.emplace_back(serverData);
            }
        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {
            return createIndex(row, column, nullptr);
        }
        QModelIndex parent(const QModelIndex &child) const override
        {
            return QModelIndex();
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return m_serverList.size();
        }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return 1;
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            if (role == Qt::DisplayRole)
            {
                if (!m_serverList[index.row()].expired())
                {
                    auto pServerData = m_serverList[index.row()].lock();

                    return QVariant(QString::fromStdWString(pServerData->serverName));
                }
                else
                {
                    return QVariant();
                }
            }
            else if (role & Qt::UserRole)
            {
                return QVariant::fromValue(m_serverList[index.row()]);
            }
            return QVariant();
        }

        int GetServerIndex(const std::wstring& serverName)
        {
            for (int i = 0; i < m_serverList.size(); i++)
            {
                auto& data = m_serverList[i];
                if (!data.expired() && data.lock()->serverName == serverName)
                {
                    return i;
                }
            }
            return -1;
        }

    private:
        TOPLauncherMainWindow & m_parent;
        std::vector<std::weak_ptr<db::DBServerData>> m_serverList;
    };



    TOPLauncherMainWindow::TOPLauncherMainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_pComboUserPopupList(new QListWidget(parent))
        , m_pServerListModel(nullptr)
    {
        ui.setupUi(this);

        // set custom drop list
        ui.comboUsername->setModel(m_pComboUserPopupList->model());
        ui.comboUsername->setView(m_pComboUserPopupList);

        InitData();
    }

    TOPLauncherMainWindow::~TOPLauncherMainWindow()
    {
    }

    void TOPLauncherMainWindow::InitData()
    {
        auto pAppModel = AppModel::GetInstance();

        auto& serverList = pAppModel->GetServerData();

        // fill server list
        m_pServerListModel.reset(new ServerDropListModel(*this));
        ui.comboServer->setModel(m_pServerListModel.get());

        // fill last login user info
        auto pLastLoginUser = db::LoadLastLoginUser();
        if (pLastLoginUser)
        {
            // set server
            assert(m_pServerListModel);
            int index = m_pServerListModel->GetServerIndex(pLastLoginUser->serverName);
            ui.comboServer->setCurrentIndex(index != -1 ? index : 0);

            LoadLastLoginUser(pLastLoginUser->serverName);
        }


    }

    void TOPLauncherMainWindow::InsertLoginUserItem(QListWidget* pListWidget, const std::shared_ptr<db::DBUserData> pUserData)
    {
        assert(pListWidget && pUserData);

        auto pNewItem = new QListWidgetItem();
        pNewItem->setText(QString::fromStdWString(pUserData->username));
        pNewItem->setData(Qt::UserRole, QVariant::fromValue(pUserData));
        pListWidget->addItem(pNewItem);
        pListWidget->setItemWidget(pNewItem, new LoginUserItem(pListWidget, *this, pUserData->username, pUserData->serverName));

    }

    void TOPLauncherMainWindow::on_serverSettingsChanged()
    {
        InitData();
    }

    void TOPLauncherMainWindow::LoadLastLoginUser(const std::wstring& serverName)
    {
        auto pLastLoginUser = db::LoadLastLoginUser(serverName);

        if (pLastLoginUser)
        {
            int lastLoginUserIndex = ui.comboUsername->findText(QString::fromStdWString(pLastLoginUser->username));
            ui.comboUsername->setCurrentIndex(lastLoginUserIndex != -1 ? lastLoginUserIndex : 0);

            ui.comboUsername->setCurrentText(QString::fromStdWString(pLastLoginUser->username));
            if (pLastLoginUser->savePassword)
            {
                ui.chkRememberPasswd->setChecked(true);
                ui.editPassword->setText(QString::fromStdWString(pLastLoginUser->password));
            }
        }
    }

    void TOPLauncherMainWindow::ReloadServerData(const std::wstring& serverName)
    {
        ui.comboUsername->clear();
        ui.editPassword->setText("");
        ui.chkRememberPasswd->setChecked(false);

        // refill user info from this server
        std::vector<std::shared_ptr<db::DBUserData>> userList;
        db::LoadUsersFromServer(serverName, userList);

        for (const std::shared_ptr<db::DBUserData>& pUser : userList)
        {
            InsertLoginUserItem(m_pComboUserPopupList, pUser);
        }

        // last login user from this server
        LoadLastLoginUser(serverName);
    }

    void TOPLauncherMainWindow::ReloadUserData(const std::wstring& username, const std::wstring& serverName)
    {
        // refill user profile
        auto pUserData = db::LoadLoginUser(username, serverName);
        assert(pUserData);

        if (pUserData)
        {
            ui.comboUsername->setCurrentText(QString::fromStdWString(pUserData->username));
            ui.editPassword->setText(QString::fromStdWString(pUserData->password));
            ui.chkRememberPasswd->setChecked(pUserData->savePassword);
        }
        else
        {
            ui.comboUsername->setCurrentText(QStringLiteral(""));
            ui.editPassword->setText(QStringLiteral(""));
            ui.chkRememberPasswd->setChecked(false);
        }
    }

    void TOPLauncherMainWindow::on_btnExit_clicked()
    {
        QApplication::exit();
    }

    void TOPLauncherMainWindow::on_btnLogin_clicked()
    {
        if (ui.comboUsername->currentText().isEmpty() ||
            ui.editPassword->text().isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Username & password input box should not empty."));
            return;
        }

        auto serverDataRef = ui.comboServer->currentData().value<std::weak_ptr<db::DBServerData>>();
        assert(!serverDataRef.expired());
        if (serverDataRef.expired())
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid server."));
            return;
        }

        auto serverData = serverDataRef.lock();

        std::wstring password = ui.editPassword->text().toStdWString();

        db::DBUserData userData;
        userData.username = ui.comboUsername->currentText().toStdWString();
        userData.serverName = serverData->serverName;
        userData.savePassword = ui.chkRememberPasswd->isChecked();
        if (userData.savePassword)
        {
            userData.password = password;
        }
        userData.lastLoginTime = util::Time::Now();

        db::SaveLoginUser(userData);


        // TODO: start the game
        std::wstring serverAddress = serverData->serverAddress;

    }

    void TOPLauncherMainWindow::on_btnSettings_clicked()
    {
        DlgSettings dlg(this);
        dlg.setModal(true);
        dlg.exec();
    }
    void TOPLauncherMainWindow::on_btnLanguage_clicked()
    {
        DlgLanguage dlg(this);
        dlg.setModal(true);
        dlg.exec();
    }

    void TOPLauncherMainWindow::on_btnRegister_clicked()
    {
        // Open registration 
        // http://tetrisonline.pl/top/register.php
        HWND windowHandle = HWND(this->window()->winId());
        ShellExecuteW(windowHandle, L"open", L"http://tetrisonline.pl/top/register.php", NULL, NULL, SW_SHOWNORMAL);
    }

    void TOPLauncherMainWindow::on_btnOpenForum_clicked()
    {
        // Open forum URL
        // http://tetrisonline.pl/forum/
        HWND windowHandle = HWND(this->window()->winId());
        ShellExecuteW(windowHandle, L"open", L"http://tetrisonline.pl/forum/", NULL, NULL, SW_SHOWNORMAL);
    }

    void TOPLauncherMainWindow::on_comboServer_currentIndexChanged(int index)
    {
        // refill user list
        auto pServerDataValue = ui.comboServer->currentData();
        
        auto pServerDataRef = pServerDataValue.value<std::weak_ptr<db::DBServerData>>();
        assert(!pServerDataRef.expired());
        if (!pServerDataRef.expired())
        {
            ReloadServerData(pServerDataRef.lock()->serverName);
        }

    }

    void TOPLauncherMainWindow::on_comboUsername_currentIndexChanged(int index)
    {
        // refill user settings
        auto pLoginUserItem = dynamic_cast<LoginUserItem*>(m_pComboUserPopupList->itemWidget(m_pComboUserPopupList->currentItem()));
        if (pLoginUserItem)
        {
            ReloadUserData(pLoginUserItem->username(), pLoginUserItem->serverName());
        }

    }

    void TOPLauncherMainWindow::on_comboUsername_currentTextChanged(QString text)
    {
        // clear user profile
        ui.editPassword->setText(QStringLiteral(""));
        ui.chkRememberPasswd->setChecked(false);
    }
}



