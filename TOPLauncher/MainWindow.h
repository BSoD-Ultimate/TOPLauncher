/*
 * Main window
*/
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

namespace TOPLauncher
{
    class TOPLauncherMainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        TOPLauncherMainWindow(QWidget *parent = Q_NULLPTR);
        ~TOPLauncherMainWindow();

    private:

    public slots:

    private slots:

    private:
        Ui::TOPLauncherClass ui;
    };
}

