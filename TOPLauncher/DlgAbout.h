#pragma once

#include <QDialog>
#include "ui_DlgAbout.h"

#include <memory>

namespace TOPLauncher
{
    class DlgAbout : public QDialog
    {
        Q_OBJECT

    public:
        DlgAbout(QWidget *parent = Q_NULLPTR);
        ~DlgAbout();

    private:
        void InitVersionString();
    private:
        Ui::DlgAbout ui;

    };

}

