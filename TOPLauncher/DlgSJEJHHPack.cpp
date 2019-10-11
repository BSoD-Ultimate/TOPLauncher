#include "stdafx.h"
#include "DlgSJEJHHPack.h"

#include "AppModel.h"
#include "DlgProgress.h"

#include "SjeJhhUtil.h"

#include <QThread>
#include <QFileDialog>
#include <QMessageBox>

namespace TOPLauncher
{
    class ArchivePackThread : public QThread
    {
        Q_OBJECT
    public:
        ArchivePackThread(const QString& packFolder, const QString& internalFolderName, const QString& archivePath)
            : m_packPath(packFolder)
            , m_internalFolderName(internalFolderName)
            , m_archivePath(archivePath)
            , m_retValue(0)
        {
        }

        bool GetPackResult(int& retValue)
        {
            if (isRunning())
            {
                return false;
            }

            retValue = m_retValue;
            return true;
        }

    signals:
        void progressTextUpdated(QString text);
        void progressValueUpdated(int value);

    private:

        static int PackCallback(sjejhh_pack_file_info* pFileInfo, size_t curIndex, size_t totalCount, void* userdata)
        {
            ArchivePackThread* pThis = reinterpret_cast<ArchivePackThread*>(userdata);

            switch (pFileInfo->fileType)
            {
            case SJEJHH_PACK_FILETYPE_FILEPATH:
            {
                std::wstring filename(pFileInfo->filename, pFileInfo->filenameLength);
                emit pThis->progressTextUpdated(QString(tr("Packing file \"%1\" ...")).arg(QString::fromStdWString(filename)));
            }
                break;
            case SJEJHH_PACK_FILETYPE_MEMORYDATA:
            {
                emit pThis->progressTextUpdated(tr("Packing Memory data..."));
            }
                break;
            default:
                break;
            }

            emit pThis->progressValueUpdated(100 * curIndex / totalCount);

            return 1;
        }

        void run() override
        {
            struct D
            {
                void operator()(sjejhh_pack_context* ptr)
                {
                    sjejhh_pack_close(ptr);
                }
            };
            std::unique_ptr<sjejhh_pack_context, D> packContext;
            packContext.reset(sjejhh_pack_create_file(m_internalFolderName.toUtf8().constData(), m_archivePath.toStdWString().c_str()));

            std::error_code code;
            filesystem::directory_iterator dirIter(m_packPath.toStdWString(), code);

            while (dirIter != filesystem::end(dirIter))
            {
                std::wstring file = dirIter->path();
                sjejhh_pack_add_file(packContext.get(), file.c_str());
                dirIter++;
            }

            m_retValue = sjejhh_pack_do_pack(packContext.get(), PackCallback, this);

        }

    private:
        QString m_archivePath;
        QString m_packPath;
        QString m_internalFolderName;

        int m_retValue;
    };


	DlgSJEJHHPack::DlgSJEJHHPack(QWidget *parent)
        : QDialog(parent)
    {
        ui.setupUi(this);

    }

	DlgSJEJHHPack::~DlgSJEJHHPack()
    {
    }


    void DlgSJEJHHPack::setArchiveFilePath(const QString& name)
    {
        ui.editArchivePath->setText(name);
    }
    void DlgSJEJHHPack::setInternalFolderName(const QString& name)
    {
        ui.editInternalFolderName->setText(name);
    }
    void DlgSJEJHHPack::setPackFolder(const QString& folder)
    {
        ui.editPackingFolder->setText(folder);
    }

    void DlgSJEJHHPack::on_btnBrowseFolder_clicked()
    {
        QString curExtractDir = ui.editPackingFolder->text();
        QString packDir = QFileDialog::getExistingDirectory(this,
            tr("Select the directory whose files will be packed..."),
            curExtractDir);

        if (!packDir.isEmpty())
        {
            ui.editPackingFolder->setText(packDir);
        }

    }

    void DlgSJEJHHPack::on_btnBrowseArchive_clicked()
    {
        QString curArchiveDir = ui.editArchivePath->text();
        QString archivePath = QFileDialog::getSaveFileName(this, tr("Set the archive's name which is going to be saved..."), curArchiveDir, tr("\"SJE.JHH\" Archive (SJE.JHH)"));

        if (!archivePath.isEmpty())
        {
            ui.editArchivePath->setText(archivePath);
        }
    }

    void DlgSJEJHHPack::accept()
    {
        // do unpack & open the progress dialog
        QString packPath = ui.editPackingFolder->text();
        QString internalFolderName = ui.editInternalFolderName->text();
        QString archivePath = ui.editArchivePath->text();

        if (packPath.isEmpty())
        {
            QMessageBox::information(this, tr("Info"), tr("Must specify a folder."));
            return;
        }
        if (internalFolderName.isEmpty())
        {
            QMessageBox::information(this, tr("Info"), tr("An internal folder name is required."));
            return;
        }
        if (archivePath.isEmpty())
        {
            QMessageBox::information(this, tr("Info"), tr("Must specify a path to save the file."));
            return;
        }

        DlgProgress progressDlg(this, tr("Packing the archive..."));

        ArchivePackThread t(packPath, internalFolderName, archivePath);

        connect(&t, &ArchivePackThread::progressTextUpdated, &progressDlg, &DlgProgress::UpdateProgressDescription);
        connect(&t, &ArchivePackThread::progressValueUpdated, &progressDlg, &DlgProgress::UpdateProgressValue);
        connect(&t, &ArchivePackThread::finished, &progressDlg, &QDialog::accept);

        t.start();
        progressDlg.exec();

        int packRet = 0;
        t.GetPackResult(packRet);
        if (packRet == SJEJHH_PACK_OK)
        {
            QMessageBox::information(this, tr("Info"), tr("Successfully packed the archive \"%1\".").arg(archivePath));
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Packaging procedure failed.").arg(archivePath));
        }


        QDialog::accept();
    }

}

#include "DlgSJEJHHPack.moc"
