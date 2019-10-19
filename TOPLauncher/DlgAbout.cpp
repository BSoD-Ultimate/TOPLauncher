#include "stdafx.h"
#include "DlgAbout.h"

namespace TOPLauncher
{
    DlgAbout::DlgAbout(QWidget *parent)
        : QDialog(parent)
    {
        ui.setupUi(this);
        InitVersionString();
    }

    DlgAbout::~DlgAbout()
    {
        
    }

    void DlgAbout::InitVersionString()
    {
        int ver1 = 0, ver2 = 0, ver3 = 0, ver4 = 0;
        util::GetCurrentExeVersion(ver1, ver2, ver3, ver4);
        QString verText = QString("%1.%2.%3")
            .arg(QString::fromStdString(std::to_string(ver1)), QString::fromStdString(std::to_string(ver2)), QString::fromStdString(std::to_string(ver3)));

        QString verString = ui.labelDesc->text();

        ui.labelDesc->setText(verString.arg(verText));
    }

}

