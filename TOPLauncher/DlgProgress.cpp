#include "stdafx.h"
#include "DlgProgress.h"

namespace TOPLauncher
{
	DlgProgress::DlgProgress(QWidget *parent, const QString& title)
        : QDialog(parent)
        , m_progressTitle(title)
    {
        ui.setupUi(this);
        setWindowFlag(Qt::WindowCloseButtonHint, false);
        setWindowTitle(title);
    }

	DlgProgress::~DlgProgress()
    {
    }

	void DlgProgress::UpdateProgressDescription(const QString& desc)
	{
        ui.labelProgressDesc->setText(desc);
	}

	void DlgProgress::UpdateProgressValue(int value)
	{
        ui.progressBar->setValue(value);
	}

}

