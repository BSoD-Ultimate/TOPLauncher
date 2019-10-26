/*
 * Main window
*/
#pragma once

#include <QtWidgets/QMainWindow>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>

#include "ui_MainWindow.h"

namespace TOPLauncher
{
    class MainWidget;
    class SettingsWidget;
    class OverlayEffectWidget;

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

        QPoint GetSettingPanelShowLocation() const;
        QPoint GetSettingPanelHideLocation() const;

        void ShowSettingPanel();
        void HideSettingPanel();

        void SetRandomBackgroundImage();

    private:
        // QWidget 
        void resizeEvent(QResizeEvent *event) override;


    public slots:

    private slots:

    private:
        Ui::TOPLauncherClass ui;
        MainWidget* m_pMainWidget;
        OverlayEffectWidget* m_pOverlayWidget;
        SettingsWidget* m_pSettingsWidget;

        QParallelAnimationGroup* m_pSettingWidgetAnimation;

        static const qreal OverlayWidgetOpacity;
    };
}

