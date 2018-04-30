#include "stdafx.h"
#include "guiUtil.h"

namespace TOPLauncher
{
    namespace util
    {
        void CenterWidgets(QWidget* widget, QWidget* host)
        {
            if (!host)
                host = widget->parentWidget();

            if (host)
            {
                auto hostRect = host->geometry();
                widget->move(hostRect.center() - widget->rect().center());
            }
            else {
                QRect screenGeometry = QApplication::desktop()->screenGeometry();
                int x = (screenGeometry.width() - widget->width()) / 2;
                int y = (screenGeometry.height() - widget->height()) / 2;
                widget->move(x, y);
            }
        }
    }
}


