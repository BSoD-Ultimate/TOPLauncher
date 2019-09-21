#include "stdafx.h"
#include "OverlayEffectWidget.h"

namespace TOPLauncher
{

    OverlayEffectWidget::OverlayEffectWidget(QWidget* parent)
        : QWidget(parent)
        , m_darkOpacity(0.7)
    {
    }
    OverlayEffectWidget::~OverlayEffectWidget()
    {
    }
    qreal OverlayEffectWidget::DarkOpacity() const
    {
        return m_darkOpacity;
    }
    void OverlayEffectWidget::SetDarkOpacity(qreal opacity)
    {
        m_darkOpacity = opacity;
    }
    void OverlayEffectWidget::mousePressEvent(QMouseEvent * e)
    {
        emit OverlayAreaClicked();
        return QWidget::mousePressEvent(e);
    }
    void OverlayEffectWidget::paintEvent(QPaintEvent * e)
    {
        QWidget::paintEvent(e);
        QPainter p(this);

        QBrush black(QColor(0, 0, 0));
        p.setBrush(black);
        p.setOpacity(m_darkOpacity);
        p.fillRect(geometry(), Qt::BrushStyle::SolidPattern);
    }

    void OverlayEffectWidget::resizeEvent(QResizeEvent * e)
    {
        return QWidget::resizeEvent(e);
    }
}