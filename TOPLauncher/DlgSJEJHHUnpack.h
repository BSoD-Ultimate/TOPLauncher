#pragma once

#include <QDialog>
#include "ui_DlgSJEJHHUnpack.h"

struct sjejhh_unpack_context;

namespace TOPLauncher
{

    class DlgSJEJHHUnpack : public QDialog
    {
        Q_OBJECT

    public:
        DlgSJEJHHUnpack(QWidget* parent = Q_NULLPTR);
        ~DlgSJEJHHUnpack();

    signals:
        void ArchiveUnpackFinished(QString archivePath, QString extractPath, QString internalFolderName);

    private slots:
        void on_btnBrowseArchive_clicked();
        void on_btnExtractFolder_clicked();

        void accept() override;

    private:
        Ui::DlgSJEJHHUnpack ui;

        std::shared_ptr<sjejhh_unpack_context> m_archive;
    };

}

