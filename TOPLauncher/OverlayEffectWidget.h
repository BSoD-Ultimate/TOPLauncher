#pragma once

#include <QtWidgets/QWidget>

namespace TOPLauncher
{
    /*
     * A widget that darkens a specific area of the parent widget
    */
    class OverlayEffectWidget : public QWidget
    {
        Q_OBJECT

    public:
        OverlayEffectWidget(QWidget* parent = Q_NULLPTR);
        ~OverlayEffectWidget();

        qreal DarkOpacity() const;
        void SetDarkOpacity(qreal opacity);

    signals:
        void OverlayAreaClicked();

    private:
        // QWidget
        void paintEvent(QPaintEvent *e) override;
        void resizeEvent(QResizeEvent *e) override; 
        void mousePressEvent(QMouseEvent *e) override;

    private:
        qreal m_darkOpacity;
    };
}