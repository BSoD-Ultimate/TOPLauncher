#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QObject>

#include "AppModel.h"
#include "LanguageModel.h"

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

static bool InstallTranslation()
{
    using namespace TOPLauncher;
    auto pLangModel = LanguageModel::GetInstance();
    auto pAppModel = AppModel::GetInstance();

    auto langId = pAppModel->GetDisplayLanguage();

    UITranslator* pTranslator = nullptr;
    bool translationFound = pLangModel->FindTranslator(langId, &pTranslator);

    if (translationFound)
    {
        return qApp->installTranslator(pTranslator);
    }
    else
    {
        return qApp->installTranslator(pLangModel->GetDefaultLanguageTranslator());
    }

}

static bool InitUIConfig()
{
    using namespace TOPLauncher;

    // set style & translations
    qApp->setStyle(QStyleFactory::create("fusion"));
    QFont uiFont(QString("Calibri"), 10, QFont::Normal);
    qApp->setFont(uiFont);
    QFont::insertSubstitution(QString("Calibri"), QString("Microsoft YaHei"));
    QFont::insertSubstitution(QString("Calibri"), QString("SimSun"));

    return true;
}

int main(int argc, char *argv[])
{
    using namespace TOPLauncher;

    QApplication a(argc, argv);

    // init models
    QString error;
    if (!InitModelInstances(&error))
    {
        // could not determine translation since initialization fails, English only.
        QString errorText = QString(
            "A fatal error occurred during the initialization: \r\n"
            "%1 \r\n"
            "\r\n"
            "The program will now exit."
        ).arg(error);
        QMessageBox::critical(NULL, QString("Fatal Error"), errorText);

        return 1;
    }

    // set translation
    InstallTranslation();

    auto pAppModel = AppModel::GetInstance();
    // init app model data
    if (!pAppModel->InitModelData(&error))
    {
        QString errorText = QObject::tr(
            "A fatal error has occurred: \r\n"
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
