#include "stdafx.h"
#include "LanguageModel.h"

#include <QFile>

#include <unordered_map>
#include <algorithm>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

namespace std 
{
    template<> struct hash<QString>
    {
        std::size_t operator()(const QString& s) const
        {
            return qHash(s);
        }
    };
}

namespace TOPLauncher
{
    struct TranslationData
    {
        QString languageId;
        QString languageShowName;
        std::unordered_map<QString, QString> translationMap;

        static std::unique_ptr<TranslationData> TranslationDataFromJSONObject(const rapidjson::Value& v)
        {
            auto pTranslation = std::make_unique<TranslationData>();

            if (v.HasMember("langid") && v["langid"].IsString())
            {
                pTranslation->languageId = QString::fromStdString(v["langid"].GetString());
            }
            if (v.HasMember("langShowName") && v["langShowName"].IsString())
            {
                pTranslation->languageShowName = QString::fromStdString(v["langShowName"].GetString());
            }

            if (pTranslation->languageId.isEmpty() || pTranslation->languageShowName.isEmpty())
            {
                return nullptr;
            }

            if (v.HasMember("translations") && v["translations"].IsObject())
            {
                auto translationObject = v["translations"].GetObject();
                for (auto iter = translationObject.MemberBegin(); iter != translationObject.MemberEnd(); iter++)
                {
                    QString translationKey;
                    QString translationValue;

                    translationKey = QString::fromUtf8(iter->name.GetString(), iter->name.GetStringLength());
                    assert(iter->value.IsString());
                    translationValue = QString::fromUtf8(iter->value.GetString(), iter->value.GetStringLength());


                    pTranslation->translationMap[translationKey] = translationValue;
                }
            }


            return pTranslation;
        }
    };

    UITranslator::UITranslator(std::unique_ptr<TranslationData>&& translation)
        : m_translationData(std::forward<std::unique_ptr<TranslationData>&&>(translation))
    {
    }

    UITranslator::~UITranslator()
    {
    }

    QString UITranslator::langId() const
    {
        return m_translationData->languageId;
    }

    QString UITranslator::langShowName() const
    {
        return m_translationData->languageShowName;
    }

    QString UITranslator::translate(const char * context, const char * sourceText, const char * disambiguation, int n) const
    {
        const auto& translationMap = m_translationData->translationMap;
        if (translationMap.find(QString(disambiguation)) != translationMap.cend())
        {
            return translationMap.at(QString(disambiguation));
        }
        return QString(sourceText);
    }

    std::shared_ptr<LanguageModel> LanguageModel::GetInstance()
    {
        static std::shared_ptr<LanguageModel> instance(new LanguageModel());
        return instance;
    }

    LanguageModel::LanguageModel()
        : m_defaultLanguage("en-US")
    {
        LoadTranslationData();
    }
    LanguageModel::~LanguageModel()
    {
    }

    QString LanguageModel::GetDefaultLanguage() const
    {
        return m_defaultLanguage;
    }

    UITranslator* LanguageModel::GetDefaultLanguageTranslator() const
    {
        UITranslator* pTranslator = nullptr;
        FindTranslator(m_defaultLanguage, &pTranslator);

        return pTranslator;
    }

    std::vector<QString> LanguageModel::GetAvailableLanguages() const
    {
        std::vector<QString> languages;
        for (const auto& translator : m_translatorContainer)
        {
            languages.push_back(translator->langId());
        }
        return languages;
    }

    UITranslator * LanguageModel::GetTranslatorAt(int index) const
    {
        return m_translatorContainer[index].get();
    }

    bool LanguageModel::FindTranslator(const QString & langId, UITranslator ** translator, int * foundIndex) const
    {
        auto iter = std::find_if(m_translatorContainer.cbegin(), m_translatorContainer.cend(),
            [&langId](const auto& item)
        {
            return item->langId() == langId;
        });

        if (iter != m_translatorContainer.cend())
        {
            if (translator) *translator = iter->get();
            if (foundIndex) *foundIndex = iter - m_translatorContainer.cbegin();
            return true;
        }

        return false;
    }

    void LanguageModel::LoadTranslationData()
    {
        QByteArray translationContent;

        // load translation from resource
        {
            QFile translationFile(":/translations.json");
            translationFile.open(QIODevice::ReadOnly);

            translationContent = translationFile.readAll();

            translationFile.close();
        }

        rapidjson::Document translationJSON;
        if (translationJSON.Parse(translationContent.constData(), translationContent.length()).HasParseError())
        {
            assert(false);
            throw std::runtime_error("Translation data has semantic issues.");
        }

        assert(translationJSON.IsObject());

        if (translationJSON.HasMember("default") && translationJSON["default"].IsString())
        {
            m_defaultLanguage = QString::fromStdString(translationJSON["default"].GetString());
        }

        if (translationJSON.HasMember("languages") && translationJSON["languages"].IsArray())
        {
            auto languagesArray = translationJSON["languages"].GetArray();
            for (int i = 0; i < languagesArray.Size(); i++)
            {
                const auto& translationObject = languagesArray[i];
                if (translationObject.IsObject())
                {
                    auto translationData = TranslationData::TranslationDataFromJSONObject(translationObject);
                    assert(translationData);
                    if (translationData)
                        m_translatorContainer.emplace_back(std::make_unique<UITranslator>(std::move(translationData)));
                }
            }
        }

        // check if default language is available
        auto iter = std::find_if(m_translatorContainer.cbegin(), m_translatorContainer.cend(),
            [this](const auto& item)
        {
            return item->langId() == m_defaultLanguage;
        });

        if (iter == m_translatorContainer.cend())
        {
            assert(false);
            throw std::runtime_error("Translation data for default language is unavailable.");
        }

    }

}