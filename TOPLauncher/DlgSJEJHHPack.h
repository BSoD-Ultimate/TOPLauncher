#pragma once

#include <QDialog>
#include "ui_DlgSJEJHHPack.h"

#include <memory>

namespace TOPLauncher
{
    class DlgSJEJHHPack : public QDialog
    {
        Q_OBJECT

    public:
		DlgSJEJHHPack(QWidget *parent = Q_NULLPTR);
        ~DlgSJEJHHPack();

    private:
        Ui::DlgSJEJHHPack ui;
    };

}

