#include "stdafx.h"
#include "MainWindow.h"
#include "LoginUserItem.h"

#include "DlgLanguage.h"
#include "MainWidget.h"
#include "SettingsWidget.h"

#include "AppModel.h"

#include "dbUser.h"



namespace TOPLauncher
{
    TOPLauncherMainWindow::TOPLauncherMainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_pSettingsWidget(NULL)
    {
        ui.setupUi(this);
        InitUI();
        //m_pSettingsWidget = new SettingsWidget(this, ui.centralwidget);
    }

    TOPLauncherMainWindow::~TOPLauncherMainWindow()
    {
        
    }
    MainWidget * TOPLauncherMainWindow::GetMainWidget() const
    {
        return ui.mainWidget;
    }
    void TOPLauncherMainWindow::InitUI()
    {
        QRect rcMainWidget = ui.mainWidget->geometry();


    }
    void TOPLauncherMainWindow::resizeEvent(QResizeEvent * e)
    {
        auto newSize = e->size();
        ui.mainWidget->setGeometry(QRect(10, 10, newSize.width() - 20, newSize.height() - 20));
        return QMainWindow::resizeEvent(e);
    }
}



