#include "stdafx.h"
#include "DlgSJEJHHUnpack.h"

#include "AppModel.h"
#include "DlgProgress.h"

#include "SjeJhhUtil.h"

#include <QThread>
#include <QFileDialog>

namespace TOPLauncher
{
    class ArchiveUnpackThread : public QThread
    {
        Q_OBJECT
    public:
        ArchiveUnpackThread(const QString& archivePath, const QString& extractDirectory)
            : m_archivePath(archivePath)
            , m_extractPath(extractDirectory)
        {
            filesystem::path p = m_archivePath.toStdWString();
            unpackContext.reset(sjejhh_unpack_open(p.c_str()));
        }

        QString internalFolderName() const
        {
            if (!unpackContext)
            {
                return QString();
            }

            sjejhh_unpack_global_info gi = { 0 };
            sjejhh_unpack_get_global_info(unpackContext.get(), &gi);

            std::string internalFolderNameStr;
            internalFolderNameStr.assign(gi.internalFolderName, gi.internalFolderNameLength);
            
            return QString::fromStdString(internalFolderNameStr);
        }

    signals:
        void progressTextUpdated(QString text);
        void progressValueUpdated(int value);

    private:

        void run() override
        {
            if (!unpackContext)
            {
                emit progressTextUpdated(tr("Open the archive failed."));
                return;
            }

            // get global info
            sjejhh_unpack_global_info gi = { 0 };
            sjejhh_unpack_get_global_info(unpackContext.get(), &gi);

            filesystem::create_directories(m_extractPath.toStdWString());

            int fileCount = gi.fileCount;
            
            emit progressValueUpdated(0);

            for (size_t i = 0; i < gi.fileCount; i++)
            {
                sjejhh_unpack_file_info curFileInfo = { 0 };
                sjejhh_unpack_get_current_file_info(unpackContext.get(), &curFileInfo);

                std::wstring filename(curFileInfo.filename, curFileInfo.filenameLength);
                filesystem::path extractFilePath = m_extractPath.toStdWString();
                extractFilePath /= filename;

                emit progressTextUpdated(QString(tr("Unpacking file \"%1\" ...")).arg(QString::fromStdWString(filename)));

                FILE* fp = _wfopen(extractFilePath.c_str(), L"wb");

                const size_t bufSize = 1000;
                std::unique_ptr<char[]> readBuf(new char[bufSize]);
                size_t readBytes = 0;
                size_t bytesRemaining = 0;

                while (sjejhh_unpack_read_current_file(unpackContext.get(), readBuf.get(), bufSize, &readBytes, &bytesRemaining) != SJEJHH_UNPACK_EOF)
                {
                    if (!curFileInfo.isEncrypted)
                    {
                        fwrite(readBuf.get(), 1, readBytes, fp);
                    }
                    else
                    {
                        bool isEnd = bytesRemaining == 0;

                        int decryptLength = sjejhh_unpack_decrypt_read_data(unpackContext.get(), readBuf.get(), readBytes, NULL, 0);
                        std::unique_ptr<char[]> decryptedData(new char[decryptLength]);
                        sjejhh_unpack_decrypt_read_data(unpackContext.get(), readBuf.get(), readBytes, decryptedData.get(), decryptLength);
                        fwrite(decryptedData.get(), 1, decryptLength, fp);
                        fflush(fp);
                    }
                }

                fclose(fp);

                emit progressValueUpdated(100 * (i + 1) / fileCount);

                sjejhh_unpack_goto_next_file(unpackContext.get());

            }
        }

    private:
        QString m_archivePath;
        QString m_extractPath;
        QString m_internalFolderName;

        struct D
        {
            void operator()(sjejhh_unpack_context* ptr)
            {
                sjejhh_unpack_close(ptr);
            }
        };
        std::unique_ptr<sjejhh_unpack_context, D> unpackContext;
    };

    DlgSJEJHHUnpack::DlgSJEJHHUnpack(QWidget *parent)
        : QDialog(parent)
    {
        ui.setupUi(this);
    }

    DlgSJEJHHUnpack::~DlgSJEJHHUnpack()
    {
    }

    void DlgSJEJHHUnpack::on_btnBrowseArchive_clicked()
    {
        QString archivePath = QFileDialog::getOpenFileName(this, tr("Select an archive to extract..."),
            QString::fromStdWString(AppModel::GetInstance()->GetGameExecutablePath().wstring()),
            tr("\"SJE.JHH\" Archive (SJE.JHH)"), nullptr, 0);

        if (!archivePath.isEmpty())
        {
            ui.editArchivePath->setText(archivePath);
            
            // read archive internal folder name
            QString internalFolderName;
            {
                auto unpackCtx = sjejhh_unpack_open(archivePath.toStdWString().c_str());

                sjejhh_unpack_global_info gi = { 0 };
                sjejhh_unpack_get_global_info(unpackCtx, &gi);

                internalFolderName = QString::fromStdString(std::string(gi.internalFolderName, gi.internalFolderNameLength));

                sjejhh_unpack_close(unpackCtx);
            }

            internalFolderName.replace(QChar('\\'), QChar('_'));
            internalFolderName.replace(QChar('/'), QChar('_'));

            // assign extract directory
            QString extractPath;
            QString tempPath = util::GetTempDirectory();
            tempPath.replace(QChar('\\'), QChar('/'));
            extractPath = QString("%1/extracted_%2").arg(tempPath, internalFolderName);

            ui.editExtractPath->setText(extractPath);
        }
    }

    void DlgSJEJHHUnpack::on_btnExtractFolder_clicked()
    {
        QString curExtractDir = ui.editExtractPath->text();
        QString saveDir = QFileDialog::getExistingDirectory(this,
            tr("Select the directory where extracted files will be stored..."),
            curExtractDir);

        if (!saveDir.isEmpty())
        {
            ui.editExtractPath->setText(saveDir);
        }
        
    }

    void DlgSJEJHHUnpack::accept()
    {
        // do unpack & open the progress dialog
        QString archivePath = ui.editArchivePath->text();
        QString extractPath = ui.editExtractPath->text();
        DlgProgress progressDlg(this, tr("Unpacking the archive..."));

        ArchiveUnpackThread t(archivePath, extractPath);

        connect(&t, &ArchiveUnpackThread::progressTextUpdated, &progressDlg, &DlgProgress::UpdateProgressDescription);
        connect(&t, &ArchiveUnpackThread::progressValueUpdated, &progressDlg, &DlgProgress::UpdateProgressValue);
        connect(&t, &ArchiveUnpackThread::finished, &progressDlg, &QDialog::accept);

        t.start();
        progressDlg.exec();

        emit ArchiveUnpackFinished(archivePath, extractPath, t.internalFolderName());

        // open the directory
        extractPath.replace(QChar('/'), QChar('\\'));
        std::wstring cmd = L"\"" + extractPath.toStdWString() + L"\"";
        ShellExecuteW(NULL, L"open", L"explorer.exe", cmd.c_str(), NULL, SW_SHOW);

        QDialog::accept();
    }

}

#include "DlgSJEJHHUnpack.moc"
