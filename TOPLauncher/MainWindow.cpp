#include "stdafx.h"
#include "MainWindow.h"
#include "LoginUserItem.h"

#include "DlgLanguage.h"
#include "SettingsWidget.h"

#include "AppModel.h"

#include "dbUser.h"



namespace TOPLauncher
{
    TOPLauncherMainWindow::TOPLauncherMainWindow(QWidget *parent)
        : QMainWindow(parent)
    {
        ui.setupUi(this);
    }

    TOPLauncherMainWindow::~TOPLauncherMainWindow()
    {
    }
}



