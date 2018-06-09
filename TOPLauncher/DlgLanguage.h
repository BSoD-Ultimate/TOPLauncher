#pragma once

#include <QWidget>
#include "ui_DlgLanguage.h"

#include <memory>

namespace TOPLauncher
{
    class LanguageItemModel;

    class DlgLanguage : public QDialog
    {
        Q_OBJECT

    public:
        DlgLanguage(QWidget *parent = Q_NULLPTR);
        ~DlgLanguage();

    private:
        void changeEvent(QEvent* event) override;

    private slots:

        void accept() override;
        void reject() override;

        void on_listLanguages_clicked(const QModelIndex &index);
        void on_btnUseSystemLang_clicked();
        void on_okButton_clicked();
        void on_cancelButton_clicked();

    private:
        Ui::DlgLanguage ui;
        std::unique_ptr<LanguageItemModel> m_pLanguageItemModel;
    };

}

