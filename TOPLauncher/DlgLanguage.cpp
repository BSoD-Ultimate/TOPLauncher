#include "stdafx.h"
#include "DlgLanguage.h"

#include "AppModel.h"
#include "LanguageModel.h"
#include "LanguageItemModel.h"

#include <QMessageBox>

namespace TOPLauncher
{
    DlgLanguage::DlgLanguage(QWidget *parent)
        : QDialog(parent)
        , m_pLanguageItemModel(new LanguageItemModel(this))
    {
        ui.setupUi(this);
        ui.listLanguages->setModel(m_pLanguageItemModel.get());

        auto pAppModel = AppModel::GetInstance();

        ui.listLanguages->setCurrentIndex(
            m_pLanguageItemModel->index(0,0));/*util::GetLanguageIndex(pAppModel->GetDisplayLanguage()), 0) */

    }

    DlgLanguage::~DlgLanguage()
    {
    }

    void DlgLanguage::changeEvent(QEvent * event)
    {
        QDialog::changeEvent(event);
        if (QEvent::LanguageChange == event->type())
        {
            ui.retranslateUi(this);
        }
    }

    void DlgLanguage::on_btnUseSystemLang_clicked()
    {
        QString langId = util::GetSystemLanguageName();
        auto pLangModel = LanguageModel::GetInstance();

        UITranslator* pTranslator = nullptr;
        bool translatorFound = pLangModel->FindTranslator(langId, &pTranslator);

        if (translatorFound)
        {
            assert(pTranslator);

            QString langShow = pTranslator->langShowName();
            QString displayFormat = QObject::tr("Would you like to use your system language \"%1\" as the display language?");
            QString promptText = displayFormat.arg(langShow);
            if (QMessageBox::question(this, QObject::tr("Question"), promptText, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            {
                auto pAppModel = AppModel::GetInstance();
                pAppModel->SetDisplayLanguage(langId);
            }

            close();
        }
        else
        {
            QString displayFormat = QObject::tr("Could not find translations for your system language \"%1\".");
            QString promptText = displayFormat.arg(langId);
            QMessageBox::critical(this, QObject::tr("Error"), promptText);
        }
    }

    void DlgLanguage::on_okButton_clicked()
    {
    }

    void DlgLanguage::on_cancelButton_clicked()
    {
    }

    void DlgLanguage::accept()
    {
        auto langId = ui.listLanguages->currentIndex().data(Qt::UserRole).toString();
        auto pAppModel = AppModel::GetInstance();
        pAppModel->SetDisplayLanguage(langId);

        QDialog::accept();
    }

    void DlgLanguage::reject()
    {
        auto pAppModel = AppModel::GetInstance();
        //util::SetDisplayLanguage(pAppModel->GetDisplayLanguage());

        QDialog::accept();
    }

    void DlgLanguage::on_listLanguages_clicked(const QModelIndex &index)
    {
        auto langId = ui.listLanguages->currentIndex().data(Qt::UserRole).toString();

        //util::SetDisplayLanguage(langId.toStdWString());
    }
}

