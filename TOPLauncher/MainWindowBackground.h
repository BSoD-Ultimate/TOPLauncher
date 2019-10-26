#pragma once

#include <QtWidgets/QWidget>

#include "utils.h"

namespace TOPLauncher
{
    class MainWindowBackground : public QWidget
    {
        Q_OBJECT

    public:
        MainWindowBackground(QWidget* parent = Q_NULLPTR);
        ~MainWindowBackground();

        bool setShowImage(const filesystem::path& imagePath);

    private:
        void paintEvent(QPaintEvent* e) override;

    private:
        std::unique_ptr<QImage> m_imageShow;
    };
}