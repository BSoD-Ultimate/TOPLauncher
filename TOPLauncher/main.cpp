#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets>
#include <QMessageBox>
#include <QObject>

#include "AppModel.h"

namespace TOPLauncher
{
    static void InitApp()
    {
        auto pAppModel = AppModel::GetInstance();

        bool ret = pAppModel->InitAppConfig();
        if (!ret)
        {
            QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("User profile open failed, the program will now exit."));
            throw 1;
        }

        if (!pAppModel->InitGameConfig())
        {
            QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("Unable to locate game executable path, the program will now exit."));
            throw 1;
        };

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

    // set style & translations
    a.setStyle(QStyleFactory::create("fusion"));
    QFont uiFont(QString("Calibri"), 10, QFont::Normal);
    a.setFont(uiFont);
    QFont::insertSubstitution(QString("Calibri"), QString("Microsoft YaHei"));
    QFont::insertSubstitution(QString("Calibri"), QString("SimSun"));

    util::SetDisplayLanguage(pAppModel->GetDisplayLanguage());

    // init application model
    try
    {
        InitApp();
    }
    catch (const int retValue)
    {
        return retValue;
    }

    // show Main window
    TOPLauncherMainWindow w;
    w.show();
    int retValue = a.exec();

    CleanupResources();

    return retValue;
}
