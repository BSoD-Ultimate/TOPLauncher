#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QObject>

#include "AppModel.h"
#include "LanguageModel.h"

#ifdef QT_STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWbmpPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
#endif

static bool InitModelInstances(QString* errMsg)
{
    using namespace TOPLauncher;

    try
    {
        auto pAppModel = AppModel::GetInstance();
        auto pLangModel = LanguageModel::GetInstance();
        return true;
    }
    catch (const std::runtime_error& e)
    {
        if (errMsg)
        {
            *errMsg = QString::fromStdString(std::string(e.what()));
        }
        return false;
    }

}

static bool InitUIConfig()
{
    using namespace TOPLauncher;

    // set style & translations
    qApp->setStyle(QStyleFactory::create("fusion"));
    QFont uiFont(QString("Calibri"), 10, QFont::Normal);
    qApp->setFont(uiFont);
    QFont::insertSubstitution(QString("Calibri"), QString("STXihei"));
    QFont::insertSubstitution(QString("Calibri"), QString("SimHei"));

    return true;
}

int main(int argc, char *argv[])
{
    using namespace TOPLauncher;
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    // init models
    QString error;
    if (!InitModelInstances(&error))
    {
        // could not determine translation since initialization fails, English only.
        QString errorText = QString(
            "A fatal error occurred during the startup: \r\n"
            "%1 \r\n"
            "\r\n"
            "The program will now exit."
        ).arg(error);
        QMessageBox::critical(NULL, QString("Fatal Error"), errorText);

        return 1;
    }

    auto pAppModel = AppModel::GetInstance();
    // init app model data
    if (!pAppModel->InitModelData(&error))
    {
        QString errorText = QObject::tr(
            "A fatal error has occurred during the startup: \r\n"
            "%1 \r\n"
            "The program will now exit."
        ).arg(error);
        QMessageBox::critical(NULL, QObject::tr("Fatal Error"), errorText);
        return 1;
    }

    InitUIConfig();

    // show Main window
    TOPLauncherMainWindow w;
    w.show();
    int retValue = a.exec();

    return retValue;
}
