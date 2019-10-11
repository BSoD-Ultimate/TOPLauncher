#include "stdafx.h"
#include "DlgSJEJHHUnpack.h"

#include "AppModel.h"
#include "DlgProgress.h"

#include "SjeJhhUtil.h"

#include <QThread>
#include <QFileDialog>
#include <QMessageBox>

namespace TOPLauncher
{
    class ArchiveUnpackThread : public QThread
    {
        Q_OBJECT
    public:
        ArchiveUnpackThread(sjejhh_unpack_context* ctx, const QString& extractDirectory)
            : m_unpackContext(ctx)
            , m_extractPath(extractDirectory)
            , m_retValue(0)
        {
            assert(ctx);
        }

        bool GetUnpackResult(int& retValue)
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

        void run() override
        {
            // get global info
            sjejhh_unpack_global_info gi = { 0 };
            sjejhh_unpack_get_global_info(m_unpackContext, &gi);

            filesystem::create_directories(m_extractPath.toStdWString());

            int fileCount = gi.fileCount;

            emit progressValueUpdated(0);

            for (size_t i = 0; i < gi.fileCount; i++)
            {
                sjejhh_unpack_file_info curFileInfo = { 0 };
                sjejhh_unpack_get_current_file_info(m_unpackContext, &curFileInfo);

                std::wstring filename(curFileInfo.filename, curFileInfo.filenameLength);
                filesystem::path extractFilePath = m_extractPath.toStdWString();
                extractFilePath /= filename;

                emit progressTextUpdated(QString(tr("Unpacking file \"%1\" ...")).arg(QString::fromStdWString(filename)));

                FILE* fp = _wfopen(extractFilePath.c_str(), L"wb");

                const size_t bufSize = 1000;
                std::unique_ptr<char[]> readBuf(new char[bufSize]);
                size_t readBytes = 0;
                size_t bytesRemaining = 0;

                while (1)
                {
                    int readRet = sjejhh_unpack_read_current_file(m_unpackContext, readBuf.get(), bufSize, &readBytes, &bytesRemaining);

                    if (readRet == SJEJHH_UNPACK_OK)
                    {
                        if (!curFileInfo.isEncrypted)
                        {
                            fwrite(readBuf.get(), 1, readBytes, fp);
                        }
                        else
                        {
                            bool isEnd = bytesRemaining == 0;

                            int decryptLength = sjejhh_unpack_decrypt_read_data(m_unpackContext, readBuf.get(), readBytes, NULL, 0);
                            std::unique_ptr<char[]> decryptedData(new char[decryptLength]);
                            sjejhh_unpack_decrypt_read_data(m_unpackContext, readBuf.get(), readBytes, decryptedData.get(), decryptLength);
                            fwrite(decryptedData.get(), 1, decryptLength, fp);
                            fflush(fp);
                        }
                    }
                    else if (readRet == SJEJHH_UNPACK_EOF)
                    {
                        break;
                    }
                    else
                    {
                        m_retValue = readRet;
                        return;
                    }
                }

                fclose(fp);

                emit progressValueUpdated(100 * (i + 1) / fileCount);

                sjejhh_unpack_goto_next_file(m_unpackContext);

            }
        }

    private:
        sjejhh_unpack_context* m_unpackContext;
        QString m_extractPath;

        int m_retValue;
    };

    DlgSJEJHHUnpack::DlgSJEJHHUnpack(QWidget* parent)
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
            struct D
            {
                void operator()(sjejhh_unpack_context* ptr)
                {
                    sjejhh_unpack_close(ptr);
                }
            };

            // attempt to open the archive
            m_archive = std::shared_ptr<sjejhh_unpack_context>(sjejhh_unpack_open(archivePath.toStdWString().c_str()), D());

            if (!m_archive)
            {
                QMessageBox::critical(this, tr("Error"), tr("Open the archive \"%1\" failed.").arg(archivePath));
                ui.editArchivePath->setText("");
                ui.editExtractPath->setText("");
                return;
            }

            // read archive internal folder name
            QString internalFolderName;
            sjejhh_unpack_global_info gi = { 0 };
            sjejhh_unpack_get_global_info(m_archive.get(), &gi);

            internalFolderName = QString::fromStdString(std::string(gi.internalFolderName, gi.internalFolderNameLength));

            internalFolderName.replace(QChar('\\'), QChar('_'));
            internalFolderName.replace(QChar('/'), QChar('_'));

            // assign extract directory
            QString extractPath;
            QString tempPath = util::GetTempDirectory();
            tempPath.replace(QChar('\\'), QChar('/'));
            extractPath = QString("%1/extracted_%2").arg(tempPath, internalFolderName);

            ui.editArchivePath->setText(archivePath);
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

        if (archivePath.isEmpty())
        {
            QMessageBox::information(this, tr("Info"), tr("Must specify an archive file."));
            return;
        }

        if (extractPath.isEmpty())
        {
            QMessageBox::information(this, tr("Info"), tr("Must specify a folder to store extracted files."));
            return;
        }

        DlgProgress progressDlg(this, tr("Unpacking the archive..."));

        ArchiveUnpackThread t(m_archive.get(), extractPath);

        connect(&t, &ArchiveUnpackThread::progressTextUpdated, &progressDlg, &DlgProgress::UpdateProgressDescription);
        connect(&t, &ArchiveUnpackThread::progressValueUpdated, &progressDlg, &DlgProgress::UpdateProgressValue);
        connect(&t, &ArchiveUnpackThread::finished, &progressDlg, &QDialog::accept);

        t.start();
        progressDlg.exec();

        QString internalFolderName;
        sjejhh_unpack_global_info gi = { 0 };
        sjejhh_unpack_get_global_info(m_archive.get(), &gi);
        internalFolderName = QString::fromStdString(std::string(gi.internalFolderName, gi.internalFolderNameLength));

        emit ArchiveUnpackFinished(archivePath, extractPath, internalFolderName);

        // open the directory
        extractPath.replace(QChar('/'), QChar('\\'));
        std::wstring cmd = L"\"" + extractPath.toStdWString() + L"\"";
        ShellExecuteW(NULL, L"open", L"explorer.exe", cmd.c_str(), NULL, SW_SHOW);

        QDialog::accept();
    }

}

#include "DlgSJEJHHUnpack.moc"
