#pragma once

#include <QDialog>
#include "ui_DlgSJEJHHPack.h"

#include <memory>

namespace TOPLauncher
{
    class DlgSJEJHHPack : public QDialog
    {
        Q_OBJECT

    public:
		DlgSJEJHHPack(QWidget *parent = Q_NULLPTR);
        ~DlgSJEJHHPack();

        void setArchiveFilePath(const QString& name);
        void setInternalFolderName(const QString& name);
        void setPackFolder(const QString& folder);

    private slots:
        void on_btnBrowseFolder_clicked();
        void on_btnBrowseArchive_clicked();

        void accept() override;

    private:
        Ui::DlgSJEJHHPack ui;
    };

}

