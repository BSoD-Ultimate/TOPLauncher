#include "stdafx.h"
#include "TOPLauncherMainWindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets>
#include <QMessageBox>
#include <QObject>

#include "AppModel.h"

namespace TOPLauncher
{
    static bool InitApp()
    {
        auto pAppModel = AppModel::GetInstance();

        bool ret = pAppModel->InitAppConfig();

        if (!ret)
        {
            return false;
        }

    }

    static void CleanupResources()
    {

    }
}

int main(int argc, char *argv[])
{
    using namespace TOPLauncher;

    QApplication a(argc, argv);

    auto pAppModel = AppModel::GetInstance();

    if (!pAppModel->InitAppConfig())
    {
        QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("User profile open failed, the program will now exit."));
        return 1;
    };

    if (!pAppModel->InitGameConfig())
    {
        QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("Unable to locate game executable path, the program will now exit."));
        return 1;
    };

    util::SetDisplayLanguage(pAppModel->GetDisplayLanguage());

    TOPLauncherMainWindow w;
    w.show();
    int retValue = a.exec();

    CleanupResources();

    return retValue;
}
