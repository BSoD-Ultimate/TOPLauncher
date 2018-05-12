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

        static QString GetLanguageFileName(const std::wstring& langId)
        {
            if (langId == L"en-US")
            {
                return QStringLiteral("");
            }
            else if (langId == L"zh-CN")
            {
                return QStringLiteral(":/languages/toplauncher_zh.qm");
            }
            else
            {
                return QStringLiteral("");
            }
        }

        bool SetDisplayLanguage(const std::wstring& langId)
        {
            static std::shared_ptr<QTranslator> translator(new QTranslator());

            auto& availableLanguages = util::GetAvailableLanguages();

            auto iter = std::find_if(availableLanguages.cbegin(), availableLanguages.cend(),
                [&langId](const std::pair<std::wstring, std::wstring>& lang)
            {
                return lang.first == langId;
            });

            assert(iter != availableLanguages.cend());
            if (iter == availableLanguages.cend())
            {
                return false;
            }

            QString translateFilePath = GetLanguageFileName(langId);

            if (!translateFilePath.isEmpty())
            {
                translator->load(translateFilePath);
                qApp->installTranslator(translator.get());
            }
            else
            {
                qApp->removeTranslator(translator.get());
            }


            return true;
        }

    }
}


