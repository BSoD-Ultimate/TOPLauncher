#pragma once

#include <QDialog>
#include "ui_DlgSJEJHHUnpack.h"

#include <memory>

namespace TOPLauncher
{
    class DlgSJEJHHUnpack : public QDialog
    {
        Q_OBJECT

    public:
        DlgSJEJHHUnpack(QWidget* parent = Q_NULLPTR);
        ~DlgSJEJHHUnpack();

    private:
        Ui::DlgSJEJHHUnpack ui;
    };

}

