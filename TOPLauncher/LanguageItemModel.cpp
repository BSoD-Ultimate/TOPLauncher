#include "stdafx.h"
#include "LanguageItemModel.h"

#include "AppModel.h"

namespace TOPLauncher
{

    LanguageItemModel::LanguageItemModel(QObject* parent)
        : QAbstractItemModel(parent)
    {
    }

    LanguageItemModel::~LanguageItemModel()
    {
    }

    QModelIndex LanguageItemModel::index(int row, int column, const QModelIndex & parent) const
    {
        return createIndex(row, column, nullptr);
    }

    QModelIndex LanguageItemModel::parent(const QModelIndex & child) const
    {
        return QModelIndex();
    }

    int LanguageItemModel::rowCount(const QModelIndex & parent) const
    {
        return util::GetAvailableLanguages().size();
    }

    int LanguageItemModel::columnCount(const QModelIndex & parent) const
    {
        return 1;
    }

    QVariant LanguageItemModel::data(const QModelIndex & index, int role) const
    {
        if (role == Qt::DisplayRole)
        {
            if (index.row() < util::GetAvailableLanguages().size())
            {
                return QVariant(QString::fromStdWString(util::GetAvailableLanguages()[index.row()].second));
            }
            else
            {
                return QVariant();
            }
        }
        else if (role & Qt::UserRole)
        {
            if (index.row() < util::GetAvailableLanguages().size())
            {
                return QVariant(QString::fromStdWString(util::GetAvailableLanguages()[index.row()].first));
            }
            else
            {
                return QVariant();
            }
        }
        return QVariant();
    }

}

