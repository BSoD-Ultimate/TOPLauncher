#include "stdafx.h"
#include "DlgLanguage.h"

#include "AppModel.h"
#include "LanguageItemModel.h"

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
            m_pLanguageItemModel->index(util::GetLanguageIndex(pAppModel->GetDisplayLanguage()), 0));

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
        std::wstring langId = util::GetSystemPreferredLanguage();
        QString displayFormat = QObject::tr("Would you like to use your system language \"{}\" as the display language?");

        std::wstring langShow = util::GetLanguageShowString(langId);
        std::wstring tipString = util::wstring_format(displayFormat.toStdWString().c_str(), langShow);

        if (QMessageBox::question(this, QObject::tr("Question"), QString::fromStdWString(tipString), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            auto pAppModel = AppModel::GetInstance();
            pAppModel->SetDisplayLanguage(langId);

            close();
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
        pAppModel->SetDisplayLanguage(langId.toStdWString());

        QDialog::accept();
    }

    void DlgLanguage::reject()
    {
        auto pAppModel = AppModel::GetInstance();
        util::SetDisplayLanguage(pAppModel->GetDisplayLanguage());

        QDialog::accept();
    }

    void DlgLanguage::on_listLanguages_clicked(const QModelIndex &index)
    {
        auto langId = ui.listLanguages->currentIndex().data(Qt::UserRole).toString();

        util::SetDisplayLanguage(langId.toStdWString());
    }
}

