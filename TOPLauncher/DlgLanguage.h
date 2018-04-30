#pragma once

#include <QWidget>
#include "ui_DlgLanguage.h"

namespace TOPLauncher
{
    class DlgLanguage : public QDialog
    {
        Q_OBJECT

    public:
        DlgLanguage(QWidget *parent = Q_NULLPTR);
        ~DlgLanguage();

    private:
        Ui::DlgLanguage ui;
    };

}

