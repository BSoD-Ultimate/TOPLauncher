#pragma once

#include <QtWidgets>

namespace TOPLauncher
{
    namespace util
    {
        void CenterWidgets(QWidget *widget, QWidget *host = 0);

        bool SetDisplayLanguage(const std::wstring & langId);


    }
}


