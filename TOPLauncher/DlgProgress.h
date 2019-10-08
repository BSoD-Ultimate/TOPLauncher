#pragma once

#include <QDialog>
#include "ui_DlgProgress.h"

#include <memory>

namespace TOPLauncher
{

    class DlgProgress : public QDialog
    {
        Q_OBJECT

    public:
		DlgProgress(QWidget *parent = Q_NULLPTR);
        ~DlgProgress();

    public slots:

		void UpdateProgressDescription(const QString& desc);
		void UpdateProgressValue(int value);
    private:
        Ui::DlgProgress ui;

    };

}

