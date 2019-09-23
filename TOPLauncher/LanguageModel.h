#pragma once

#include <QTranslator>

namespace TOPLauncher
{
    struct TranslationData;
    class UITranslator : public QTranslator
    {
    public:
        UITranslator(std::unique_ptr<TranslationData>&& translation);
        ~UITranslator();

        UITranslator(const UITranslator&) = delete;
        UITranslator& operator=(const UITranslator&) = delete;

        QString langId() const;
        QString langShowName() const;

        QString translate(const char *context, const char *sourceText,
            const char *disambiguation = Q_NULLPTR, int n = -1) const override;

    private:
        std::unique_ptr<TranslationData> m_translationData;

    };

    class LanguageModel
    {
    private:
        LanguageModel();
    public:
        ~LanguageModel();

    public:
        static std::shared_ptr<LanguageModel> GetInstance();

    public:
        QString GetDefaultLanguage() const;
        UITranslator* GetDefaultLanguageTranslator() const;

        std::vector<QString> GetAvailableLanguages() const;
        UITranslator* GetTranslatorAt(int index) const;
        bool FindTranslator(const QString& langId, UITranslator** translator = nullptr, int* foundIndex = nullptr) const;

    private:
        void LoadTranslationData();

    private:
        QString m_defaultLanguage;
        std::vector<std::unique_ptr<UITranslator>> m_translatorContainer;

    };
}