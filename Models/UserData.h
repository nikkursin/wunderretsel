#ifndef USERDATA_H
#define USERDATA_H

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
};

#endif // USERDATA_H
