#include "stdafx.h"
#include "MainWindow.h"
#include "LoginUserItem.h"

#include "DlgLanguage.h"
#include "MainWidget.h"
#include "SettingsWidget.h"
#include "OverlayEffectWidget.h"

#include "AppModel.h"
#include "dbUser.h"

#include <QResizeEvent> 

namespace TOPLauncher
{
    const qreal TOPLauncherMainWindow::OverlayWidgetOpacity = 0.8;

    TOPLauncherMainWindow::TOPLauncherMainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_pMainWidget(NULL)
        , m_pOverlayWidget(NULL)
        , m_pSettingsWidget(NULL)
        , m_pSettingWidgetAnimation(NULL)
    {
        ui.setupUi(this);
        InitUI();

    }

    TOPLauncherMainWindow::~TOPLauncherMainWindow()
    {
    }

    MainWidget* TOPLauncherMainWindow::GetMainWidget() const
    {
        return m_pMainWidget;
    }
    SettingsWidget* TOPLauncherMainWindow::GetSettingsWidget() const
    {
        return m_pSettingsWidget;
    }
    void TOPLauncherMainWindow::InitUI()
    {
        m_pMainWidget = new MainWidget(ui.centralwidget);
        m_pOverlayWidget = new OverlayEffectWidget(ui.centralwidget);
        m_pSettingsWidget = new SettingsWidget(this, ui.centralwidget);

        m_pOverlayWidget->hide();

        m_pSettingsWidget->raise();

        auto settingWidgetGeometry = m_pSettingsWidget->geometry();
        auto settingPanelShowLocation = GetSettingPanelHideLocation();
        m_pSettingsWidget->setGeometry(QRect(settingPanelShowLocation.x(), settingPanelShowLocation.y(), settingWidgetGeometry.width(), settingWidgetGeometry.height()));

        connect(m_pSettingsWidget, &SettingsWidget::SettingButtonToggled, 
            [this](bool checked)
        {
            if (checked)
            {
                ShowSettingPanel();
            }
            else
            {
                HideSettingPanel();
            }
        });

        connect(m_pOverlayWidget, &OverlayEffectWidget::OverlayAreaClicked,
            [this]()
        {
            m_pSettingsWidget->SetSettingButtonState(false);
        });

        
    }
    QPoint TOPLauncherMainWindow::GetSettingPanelShowLocation() const
    {
        return QPoint(geometry().width() - m_pSettingsWidget->geometry().width(), 0);
    }
    QPoint TOPLauncherMainWindow::GetSettingPanelHideLocation() const
    {
        return QPoint(geometry().width() - m_pSettingsWidget->GetScrollAreaLocationX(), 0);
    }

    void TOPLauncherMainWindow::ShowSettingPanel()
    {
        static const int duration = 200;
        QPoint startLocation = GetSettingPanelHideLocation();
        QPoint endLocation = GetSettingPanelShowLocation();
        qreal startOpacity = 0, endOpacity = OverlayWidgetOpacity;

        m_pOverlayWidget->show();

        if (m_pSettingWidgetAnimation)
        {
            startLocation = m_pSettingsWidget->geometry().topLeft();
            startOpacity = m_pOverlayWidget->DarkOpacity();

            m_pSettingWidgetAnimation->stop();
        }

        // overlay widget
        auto overlayWidgetAnimation = new QVariantAnimation(this);
        overlayWidgetAnimation->setStartValue(startOpacity);
        overlayWidgetAnimation->setEndValue(endOpacity);
        overlayWidgetAnimation->setDuration(duration);

        connect(overlayWidgetAnimation, &QVariantAnimation::valueChanged,
            [this](const QVariant &value) 
        {
            m_pOverlayWidget->SetDarkOpacity(value.value<qreal>());
            repaint();
        });

        // setting panel
        auto settingPanelAnimation = new QVariantAnimation(this);
        settingPanelAnimation->setStartValue(startLocation);
        settingPanelAnimation->setEndValue(endLocation);
        settingPanelAnimation->setDuration(duration);
        settingPanelAnimation->setEasingCurve(QEasingCurve(QEasingCurve::OutQuart));
        
        connect(settingPanelAnimation, &QVariantAnimation::valueChanged,
            [this](const QVariant &value)
        {
            auto location = value.value<QPoint>();
            m_pSettingsWidget->move(location);
        });

        // start animation
        m_pSettingWidgetAnimation = new QParallelAnimationGroup(this);
        m_pSettingWidgetAnimation->addAnimation(overlayWidgetAnimation);
        m_pSettingWidgetAnimation->addAnimation(settingPanelAnimation);

        connect(m_pSettingWidgetAnimation, &QAbstractAnimation::destroyed, 
            [this]()
        {
            m_pSettingWidgetAnimation = nullptr;
        });

        m_pSettingWidgetAnimation->start(QAbstractAnimation::DeleteWhenStopped);


    }
    void TOPLauncherMainWindow::HideSettingPanel()
    {
        static const int duration = 200;
        QPoint startLocation = GetSettingPanelShowLocation();
        QPoint endLocation = GetSettingPanelHideLocation();
        qreal startOpacity = OverlayWidgetOpacity, endOpacity = 0;

        if (m_pSettingWidgetAnimation)
        {
            startLocation = m_pSettingsWidget->geometry().topLeft();
            startOpacity = m_pOverlayWidget->DarkOpacity();

            m_pSettingWidgetAnimation->stop();
        }

        // overlay widget
        auto overlayWidgetAnimation = new QVariantAnimation(this);
        overlayWidgetAnimation->setStartValue(startOpacity);
        overlayWidgetAnimation->setEndValue(endOpacity);
        overlayWidgetAnimation->setDuration(duration);

        connect(overlayWidgetAnimation, &QVariantAnimation::valueChanged,
            [this](const QVariant &value)
        {
            m_pOverlayWidget->SetDarkOpacity(value.value<qreal>());
            repaint();
        });
        connect(overlayWidgetAnimation, &QVariantAnimation::finished,
            [this]()
        {
            m_pOverlayWidget->hide();
        });


        // setting panel
        auto settingPanelAnimation = new QVariantAnimation(this);
        settingPanelAnimation->setStartValue(startLocation);
        settingPanelAnimation->setEndValue(endLocation);
        settingPanelAnimation->setDuration(duration);
        settingPanelAnimation->setEasingCurve(QEasingCurve(QEasingCurve::OutQuart));

        connect(settingPanelAnimation, &QVariantAnimation::valueChanged,
            [this](const QVariant &value)
        {
            auto location = value.value<QPoint>();
            m_pSettingsWidget->move(location);
        });

        // start animation
        m_pSettingWidgetAnimation = new QParallelAnimationGroup(this);
        m_pSettingWidgetAnimation->addAnimation(overlayWidgetAnimation);
        m_pSettingWidgetAnimation->addAnimation(settingPanelAnimation);

        connect(m_pSettingWidgetAnimation, &QAbstractAnimation::destroyed,
            [this]()
        {
            m_pSettingWidgetAnimation = nullptr;
        });

        m_pSettingWidgetAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    }
    void TOPLauncherMainWindow::resizeEvent(QResizeEvent * e)
    {
        auto newSize = e->size();
        // main widget
        m_pMainWidget->setGeometry(QRect(10, 10, newSize.width() - 20, newSize.height() - 20));
        
        // overlay widget
        m_pOverlayWidget->setGeometry(geometry());

        // setting widget
        auto settingWidgetGeometry = m_pSettingsWidget->geometry();
        settingWidgetGeometry.setHeight(newSize.height());
        m_pSettingsWidget->setGeometry(settingWidgetGeometry);

        return QMainWindow::resizeEvent(e);
    }
}



