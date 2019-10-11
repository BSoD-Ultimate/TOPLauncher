#include "stdafx.h"
#include "MainWidget.h"
#include "LoginUserItem.h"

#include "DlgLanguage.h"
#include "DlgAbout.h"
#include "SettingsWidget.h"

#include "AppModel.h"

#include "dbUser.h"

#include <QMessageBox>
#include <QKeyEvent>

namespace TOPLauncher
{
    Q_DECLARE_METATYPE(std::weak_ptr<ServerData>);
    Q_DECLARE_METATYPE(std::shared_ptr<db::DBUserData>);

    class ServerDropListModel : public QAbstractItemModel
    {
    public:
        ServerDropListModel(MainWidget& parent)
            : m_parent(parent)
        {
            auto pAppModel = AppModel::GetInstance();
            auto& serverList = pAppModel->GetServerData();

            for (const std::shared_ptr<ServerData>& serverData : serverList)
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

                    return QVariant(pServerData->serverName);
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

        int GetServerIndex(const QString& serverName)
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
        MainWidget & m_parent;
        std::vector<std::weak_ptr<ServerData>> m_serverList;
    };



    MainWidget::MainWidget(QWidget *parent)
        : QWidget(parent)
        , m_pComboUserPopupList(new QListWidget(parent))
        , m_pServerListModel(nullptr)
    {
        ui.setupUi(this);

        // set custom drop list
        ui.comboUsername->setModel(m_pComboUserPopupList->model());
        ui.comboUsername->setView(m_pComboUserPopupList);

        ui.btnLogin->setFocus();

        InitData();
    }

    MainWidget::~MainWidget()
    {
    }

    void MainWidget::InitData()
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

    void MainWidget::InsertLoginUserItem(QListWidget* pListWidget, const std::shared_ptr<db::DBUserData> pUserData)
    {
        assert(pListWidget && pUserData);

        auto pNewItem = new QListWidgetItem();
        pNewItem->setText(pUserData->username);
        pNewItem->setData(Qt::UserRole, QVariant::fromValue(pUserData));
        pListWidget->addItem(pNewItem);
        pListWidget->setItemWidget(pNewItem, new LoginUserItem(pListWidget, *this, pUserData->username, pUserData->serverName));

    }

    void MainWidget::changeEvent(QEvent* event)
    {
        QWidget::changeEvent(event);
        if (QEvent::LanguageChange == event->type())
        {
            ui.retranslateUi(this);
        }
    }

    void MainWidget::keyPressEvent(QKeyEvent* e)
    {
        auto keyCode = e->key();
        if (keyCode == Qt::Key_Enter || keyCode == Qt::Key_Return)
        {
            on_btnLogin_clicked();
        }

        QWidget::keyPressEvent(e);
    }

    void MainWidget::on_serverSettingsChanged()
    {
        InitData();
    }

    void MainWidget::LoadLastLoginUser(const QString& serverName)
    {
        auto pLastLoginUser = db::LoadLastLoginUser(serverName);

        if (pLastLoginUser)
        {
            int lastLoginUserIndex = ui.comboUsername->findText(pLastLoginUser->username);
            ui.comboUsername->setCurrentIndex(lastLoginUserIndex != -1 ? lastLoginUserIndex : 0);

            ui.comboUsername->setCurrentText(pLastLoginUser->username);
            if (pLastLoginUser->savePassword)
            {
                ui.chkRememberPasswd->setChecked(true);
                ui.editPassword->setText(pLastLoginUser->password);
            }
        }
    }

    void MainWidget::ReloadServerData(const QString& serverName)
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

    void MainWidget::ReloadUserData(const QString& username, const QString& serverName)
    {
        // refill user profile
        auto pUserData = db::LoadLoginUser(username, serverName);
        assert(pUserData);

        if (pUserData)
        {
            ui.comboUsername->setCurrentText(pUserData->username);
            ui.editPassword->setText(pUserData->password);
            ui.chkRememberPasswd->setChecked(pUserData->savePassword);
        }
        else
        {
            ui.comboUsername->setCurrentText(QStringLiteral(""));
            ui.editPassword->setText(QStringLiteral(""));
            ui.chkRememberPasswd->setChecked(false);
        }
    }

    void MainWidget::on_btnExit_clicked()
    {
        QApplication::exit();
    }

    void MainWidget::on_btnLogin_clicked()
    {
        auto pAppModel = AppModel::GetInstance();

        if (ui.comboUsername->currentText().isEmpty() ||
            ui.editPassword->text().isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("Username & password input box should not empty."));
            return;
        }

        auto serverDataRef = ui.comboServer->currentData().value<std::weak_ptr<ServerData>>();
        assert(!serverDataRef.expired());
        if (serverDataRef.expired())
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid server profile."));
            return;
        }

        auto serverData = serverDataRef.lock();

        // check game executable
        if (pAppModel->GetGameExecutablePath().empty())
        {
            QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Could not find where the game executable locates. Startup failed."));
            return;
        }

        QString username = ui.comboUsername->currentText();
        QString password = ui.editPassword->text();

        db::DBUserData userData;
        userData.username = username;
        userData.serverName = serverData->serverName;
        userData.savePassword = ui.chkRememberPasswd->isChecked();
        if (userData.savePassword)
        {
            userData.password = password;
        }
        userData.lastLoginTime = util::Time::Now();

        db::SaveLoginUser(userData);

        ReloadServerData(serverData->serverName);

        // start the game
        {
            QString serverAddress = serverData->serverAddress;

            QString gamePath = QString::fromStdString(pAppModel->GetGameExecutablePath().u8string());

            QString startupArgs = util::GetGameStartupArgs(serverAddress, username, password);

            std::wstring startupArgFull = QString("\"%1\" %2").arg(gamePath, startupArgs).toStdWString();

            std::unique_ptr<wchar_t[]> startupArgBuf(new wchar_t[startupArgFull.length() + 1]());
            wcscpy_s(startupArgBuf.get(), startupArgFull.length() + 1, startupArgFull.c_str());

            STARTUPINFOW si = { 0 };

            PROCESS_INFORMATION pi = { 0 };

            BOOL startupSuccess = CreateProcessW(
                gamePath.toStdWString().c_str(),
                startupArgBuf.get(),
                NULL,
                NULL,
                FALSE,
                CREATE_SUSPENDED,
                NULL,
                pAppModel->GetGameDirectory().c_str(),
                &si,
                &pi);

            ResumeThread(pi.hThread);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        QApplication::exit();
    }

    void MainWidget::on_btnLanguage_clicked()
    {
        DlgLanguage dlg(this);
        dlg.setModal(true);
        dlg.exec();
    }

    void MainWidget::on_btnRegister_clicked()
    {
		// find registration URL
		std::wstring registerURL;

		auto pServerDataValue = ui.comboServer->currentData();

		auto pServerDataRef = pServerDataValue.value<std::weak_ptr<ServerData>>();
		assert(!pServerDataRef.expired());
		if (!pServerDataRef.expired())
		{
			auto pServerData = pServerDataRef.lock();
			auto& registerURLStr = pServerData->registerURL;
			if (!registerURLStr.isEmpty())
			{
				registerURL = registerURLStr.toStdWString();
			}
		}
		
		if (registerURL.empty())
		{
			assert(false);
			QMessageBox::information(this, QObject::tr("Error"), QObject::tr("Current server profile does not contain a registration URL."));
			return;
		}

        HWND windowHandle = HWND(this->window()->winId());
        ShellExecuteW(windowHandle, L"open", registerURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    void MainWidget::on_btnOpenForum_clicked()
    {
        // Open forum URL
        // http://tetrisonline.pl/forum/
        HWND windowHandle = HWND(this->window()->winId());
        ShellExecuteW(windowHandle, L"open", L"http://tetrisonline.pl/forum/", NULL, NULL, SW_SHOWNORMAL);
    }

    void MainWidget::on_btnAbout_clicked()
    {
        DlgAbout aboutDlg;
        aboutDlg.exec();
    }

    void MainWidget::on_comboServer_currentIndexChanged(int index)
    {
        // refill user list
        auto pServerDataValue = ui.comboServer->currentData();

        auto pServerDataRef = pServerDataValue.value<std::weak_ptr<ServerData>>();

        if (!pServerDataRef.expired())
        {
            ReloadServerData(pServerDataRef.lock()->serverName);
        }

    }

    void MainWidget::on_comboUsername_currentIndexChanged(int index)
    {
        // refill user settings
        auto pLoginUserItem = dynamic_cast<LoginUserItem*>(m_pComboUserPopupList->itemWidget(m_pComboUserPopupList->currentItem()));
        if (pLoginUserItem)
        {
            ReloadUserData(pLoginUserItem->username(), pLoginUserItem->serverName());
        }

    }

    void MainWidget::on_comboUsername_currentTextChanged(QString text)
    {
        // clear user profile
        ui.editPassword->setText(QStringLiteral(""));
        ui.chkRememberPasswd->setChecked(false);
    }
}



