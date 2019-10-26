#include "stdafx.h"
#include "MainWindowBackground.h"

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPaintEvent>
#include <QImageReader>

namespace TOPLauncher
{
    MainWindowBackground::MainWindowBackground(QWidget* parent)
        : QWidget(parent)
    {
    }
    MainWindowBackground::~MainWindowBackground()
    {
    }
    bool MainWindowBackground::setShowImage(const filesystem::path& imagePath)
    {
        if (imagePath.empty())
        {
            m_imageShow.reset();
            repaint();
            return true;
        }

        QString path = QString::fromStdWString(imagePath.wstring());
        QImageReader reader(path);
        reader.setAutoTransform(true);
        auto pNewImage = std::make_unique<QImage>(reader.read());
        if (pNewImage->isNull()) 
        {
            return false;
        }
        m_imageShow = std::move(pNewImage);

        repaint();

        return true;
    }
    void MainWindowBackground::paintEvent(QPaintEvent* e)
    {
        QPainter p(this);

        if (!m_imageShow)
        {
            QStyleOption o;
            o.initFrom(this);
            style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
        }
        else
        {
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
            // paint white background
            QBrush white(QColor(255, 255, 255));
            p.setBrush(white);
            p.fillRect(0, 0, width(), height(), Qt::BrushStyle::SolidPattern);


            // paint Filled image which aspect image's w/h ratio
            QSize widgetSize = size();
            QSize imageSize = m_imageShow->size();
            QSize imageScaledSize = { widgetSize.width(), widgetSize.width() * imageSize.height() / imageSize.width() };
            if (imageScaledSize.height() < widgetSize.height())
            {
                imageScaledSize = QSize(widgetSize.height() * imageSize.width() / imageSize.height(), widgetSize.height());
            }

            QPoint startPoint = { widgetSize.width() / 2 - imageScaledSize.width() / 2, widgetSize.height() / 2 - imageScaledSize.height() / 2, };
            QRect paintRect = { startPoint.x(), startPoint.y(), imageScaledSize.width(), imageScaledSize.height() };
            p.drawImage(paintRect, *m_imageShow);
            
        }

        QWidget::paintEvent(e);
    }
}