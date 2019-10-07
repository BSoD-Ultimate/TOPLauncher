#include "stdafx.h"
#include "LanguageModel.h"

#include <QDir>
#include <QFile>
#include <tinyxml2.h>

#include <unordered_map>
#include <algorithm>


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
		struct TranslationItem
		{
			QString sourceText;
			QString disambiguation;
			QString comment;
			int index = 0;
			QString translatedText;
		};
		using TranslationMap = std::unordered_map<QString, TranslationItem>;

		QString langId;
        std::unordered_map<QString, TranslationMap> translationMapData;

		static std::unique_ptr<TranslationData> TranslationDataFromXMLData(const tinyxml2::XMLDocument& d);
    };

	std::unique_ptr<TranslationData> TranslationData::TranslationDataFromXMLData(const tinyxml2::XMLDocument& d)
	{
		auto pTranslation = std::make_unique<TranslationData>();

		const tinyxml2::XMLElement* pRoot = d.RootElement();

		// get language ID
		const char* langIdValue = pRoot->Attribute("language");
		if (!langIdValue)
		{
			return nullptr;
		}
		QString langId = QString::fromUtf8(langIdValue);
		langId.replace(QChar::fromLatin1('_'), QChar::fromLatin1('-'));
		pTranslation->langId = langId;

		// parse contexts
		for (const tinyxml2::XMLElement* childContext = pRoot->FirstChildElement();
			childContext; childContext = childContext->NextSiblingElement())
		{
			QString contextName;
			TranslationMap translationMap;

			const tinyxml2::XMLElement* contextNameElement = childContext->FirstChildElement();
			if (contextNameElement->Name() != std::string("name"))
			{
				assert(false);
				return nullptr;
			}

			contextName = QString::fromUtf8(contextNameElement->GetText());

			// parse translations
			for (const tinyxml2::XMLElement* translationItem = contextNameElement->NextSiblingElement();
				translationItem; translationItem = translationItem->NextSiblingElement())
			{
				if (translationItem->Name() != std::string("message"))
				{
					assert(false);
					return nullptr;
				}

				TranslationItem item;

				for (const tinyxml2::XMLElement* trData = translationItem->FirstChildElement();
					trData; trData = trData->NextSiblingElement())
				{
					if (trData->Name() == std::string("source") && trData->GetText())
						item.sourceText = QString::fromUtf8(trData->GetText());
					if (trData->Name() == std::string("comment") && trData->GetText())
						item.disambiguation = QString::fromUtf8(trData->GetText());
					if (trData->Name() == std::string("extracomment") && trData->GetText())
						item.comment = QString::fromUtf8(trData->GetText());
					if (trData->Name() == std::string("translation") && trData->GetText())
					{
						item.translatedText = QString::fromUtf8(trData->GetText());
					}

				}

				translationMap[item.sourceText] = item;
			}

			pTranslation->translationMapData[contextName] = std::move(translationMap);

		}


		return pTranslation;
	}

    UITranslator::UITranslator(std::unique_ptr<TranslationData>&& translation)
        : m_translationData(std::forward<std::unique_ptr<TranslationData>&&>(translation))
    {
    }

    UITranslator::~UITranslator()
    {
    }

    QString UITranslator::LangId() const
    {
        return m_translationData ? m_translationData->langId : "";
    }

    QString UITranslator::LangShowName() const
    {
		std::wstring langId = LangId().toStdWString();
		
		if (langId.empty())
		{
			return QString();
		}

		int nchars = GetLocaleInfoEx(langId.c_str(), LOCALE_SNATIVELANGUAGENAME, NULL, 0);
		auto languageNameBuf = std::make_unique<wchar_t[]>(nchars);
		GetLocaleInfoEx(langId.c_str(), LOCALE_SNATIVELANGUAGENAME, languageNameBuf.get(), nchars);

		return QString::fromWCharArray(languageNameBuf.get(), nchars - 1);
    }

    TranslationData * UITranslator::GetTranslationData() const
    {
        return m_translationData.get();
    }

    bool UITranslator::isEmpty() const
    {
        return !m_translationData;
    }

    QString UITranslator::translate(const char * context, const char * sourceText, const char * disambiguation, int n) const
    {
		if (!m_translationData)
		{
			return QString(sourceText);
		}

		try
		{
			auto translationMap = m_translationData->translationMapData.at(QString::fromUtf8(context));
			const auto& translationData = translationMap.at(QString::fromUtf8(sourceText));
			if (!translationData.translatedText.isEmpty())
			{
				return translationData.translatedText;
			}
			else
			{
				return QString(sourceText);
			}
		}
		catch (const std::out_of_range&)
		{
			return QString(sourceText);
		}

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
            languages.push_back(translator->LangId());
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
            return item->LangId() == langId;
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
		static const QString translationFilePrefix = ":/translations/";
		// Open translations folder
		QDir directory(translationFilePrefix);

		QStringList translationsList = directory.entryList();

		// load translations
		for (const QString& translationFilename : translationsList)
		{
			QByteArray translationFileContent;
			{
				QString translationFilePath = translationFilePrefix + translationFilename;
				QFile translationFile(translationFilePath);
				translationFile.open(QIODevice::ReadOnly);
				translationFileContent = translationFile.readAll();
				translationFile.close();
			}

			tinyxml2::XMLDocument d;
			if (d.Parse(translationFileContent) != tinyxml2::XML_SUCCESS)
			{
				assert(false);
				throw std::runtime_error("Translation data has semantic issues.");
			}

			auto translationData = TranslationData::TranslationDataFromXMLData(d);
			assert(translationData);
			if (translationData)
				m_translatorContainer.emplace_back(std::make_unique<UITranslator>(std::move(translationData)));

		}

        // check if default language is available
        auto iter = std::find_if(m_translatorContainer.cbegin(), m_translatorContainer.cend(),
            [this](const auto& item)
        {
            return item->LangId() == m_defaultLanguage;
        });

        if (iter == m_translatorContainer.cend())
        {
            assert(false);
            throw std::runtime_error("Translation data for default language is unavailable.");
        }

    }

}