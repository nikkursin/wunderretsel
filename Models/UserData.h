#ifndef USERDATA_H
#define USERDATA_H

#include <QVector>

enum class LanguageLevel {
    A1,
    A2,
    B1,
    B2,
    C1,
    C2
};

enum class CharacterType {
    Male,
    Female,
    Mixed
};

struct UserData {
    bool isOnboardingCompleted = false;
    LanguageLevel level = LanguageLevel::A1;
    CharacterType characterType = CharacterType::Mixed;
    QVector<int> usedWordsIds;
    QVector<int> unlockedImagesIds;

    static LanguageLevel languageLevelFromString(const QString& value)
    {
        if (value == "A2") return LanguageLevel::A2;
        if (value == "B1") return LanguageLevel::B1;
        if (value == "B2") return LanguageLevel::B2;
        if (value == "C1") return LanguageLevel::C1;
        if (value == "C2") return LanguageLevel::C2;

        return LanguageLevel::A1;
    }

    static CharacterType characterTypeFromString(const QString& value)
    {
        if (value == "male") return CharacterType::Male;
        if (value == "female") return CharacterType::Female;

        return CharacterType::Mixed;
    }

    static QString languageLevelToString(LanguageLevel level)
    {
        switch (level) {
        case LanguageLevel::A1: return "A1";
        case LanguageLevel::A2: return "A2";
        case LanguageLevel::B1: return "B1";
        case LanguageLevel::B2: return "B2";
        case LanguageLevel::C1: return "C1";
        case LanguageLevel::C2: return "C2";
        }

        return "A1"; // fallback
    }

    static QString characterTypeToString(CharacterType type)
    {
        switch (type) {
        case CharacterType::Female: return "female";
        case CharacterType::Male: return "male";
        case CharacterType::Mixed: return "mixed";
        }

        return "mixed"; // fallback
    }

};



#endif // USERDATA_H
