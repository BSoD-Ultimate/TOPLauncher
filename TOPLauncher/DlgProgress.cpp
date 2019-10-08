#include "stdafx.h"
#include "DlgProgress.h"

namespace TOPLauncher
{
	DlgProgress::DlgProgress(QWidget *parent)
        : QDialog(parent)
    {
        ui.setupUi(this);

    }

	DlgProgress::~DlgProgress()
    {
    }

	void DlgProgress::UpdateProgressDescription(const QString& desc)
	{
	}

	void DlgProgress::UpdateProgressValue(int value)
	{
	}

}

