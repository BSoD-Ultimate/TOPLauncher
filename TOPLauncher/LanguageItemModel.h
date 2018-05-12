#pragma once

#include <QObject>
#include <QAbstractItemModel>

namespace TOPLauncher
{

    class LanguageItemModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        LanguageItemModel(QObject* parent = Q_NULLPTR);
        ~LanguageItemModel();

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &child) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    };

}
