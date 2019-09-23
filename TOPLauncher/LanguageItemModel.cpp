#include "stdafx.h"
#include "LanguageItemModel.h"

#include "LanguageModel.h"

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
        auto pLangModel = LanguageModel::GetInstance();
        return pLangModel->GetAvailableLanguages().size();
    }

    int LanguageItemModel::columnCount(const QModelIndex & parent) const
    {
        return 1;
    }

    QVariant LanguageItemModel::data(const QModelIndex & index, int role) const
    {
        auto pLangModel = LanguageModel::GetInstance();
        auto langList = pLangModel->GetAvailableLanguages();

        if (role == Qt::DisplayRole)
        {
            if (index.row() < langList.size())
            {
                return QVariant(pLangModel->GetTranslatorAt(index.row())->langShowName());
            }
            else
            {
                return QVariant();
            }
        }
        else if (role & Qt::UserRole)
        {
            if (index.row() < langList.size())
            {
                return QVariant(langList[index.row()]);
            }
            else
            {
                return QVariant();
            }
        }
        return QVariant();
    }

}

