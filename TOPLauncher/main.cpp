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

        bool ret = pAppModel->InitSavedData();

        if (!ret)
        {
            return false;
        }

    }
}

int main(int argc, char *argv[])
{
    using namespace TOPLauncher;

	QApplication a(argc, argv);

    auto pAppModel = AppModel::GetInstance();

    if (!pAppModel->InitSavedData())
    {
        QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("User profile open failed, the program will now exit."));
        return 1;
    };
        
    if (!pAppModel->InitGameConfig())
    {
        QMessageBox::critical(NULL, QObject::tr("Fatal Error"), QObject::tr("Unable to locate game executable path, the program will now exit."));
        return 1;
    };

	TOPLauncherMainWindow w;
	w.show();
	return a.exec();
}
