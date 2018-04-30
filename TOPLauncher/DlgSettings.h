#pragma once

#include <QtWidgets/QDialog>
#include "ui_DlgSettings.h"

class DlgSettings : public QDialog
{
    Q_OBJECT


public:
    DlgSettings(QWidget *parent = Q_NULLPTR);
    ~DlgSettings();

public:
    void foo();


signals:
    void ServerSettingsChanged();

private:
    Ui::DlgSettings ui;
};

