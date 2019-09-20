/*
 * Main window
*/
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

namespace TOPLauncher
{
    class MainWidget;
    class SettingsWidget;

    class TOPLauncherMainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        TOPLauncherMainWindow(QWidget *parent = Q_NULLPTR);
        ~TOPLauncherMainWindow();

        MainWidget* GetMainWidget() const;
        SettingsWidget* GetSettingsWidget() const;

    private:
        void InitUI();


    private:
        // QWidget 
        void resizeEvent(QResizeEvent *event) override;


    public slots:

    private slots:

    private:
        Ui::TOPLauncherClass ui;
        SettingsWidget* m_pSettingsWidget;
    };
}

