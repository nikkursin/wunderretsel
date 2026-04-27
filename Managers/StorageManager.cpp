#include "StorageManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDir>
#include <QStandardPaths>

StorageManager::StorageManager() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    m_userFilePath = dir + "/user.json";
}

UserData StorageManager::loadUserData() {
    UserData data;

    qDebug() << m_userFilePath;

    QFile file(m_userFilePath);

    if (!file.exists()) {
        return data;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return data;
    }

    const QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return data;
    }

    const QJsonObject root = doc.object();

    data.isOnboardingCompleted =
        root.value("onboardingCompleted").toBool(false);

    const QJsonObject preferences =
        root.value("preferences").toObject();

    data.level =
        languageLevelFromString(
            preferences.value("languageLevel").toString("A1")
            );

    data.characterType =
        characterTypeFromString(
            preferences.value("characterType").toString("mixed")
            );

    return data;
}

bool StorageManager::saveUser(const UserData& userData) {
    return true;
}

LanguageLevel StorageManager::languageLevelFromString(const QString& value)
{
    if (value == "A2") return LanguageLevel::A2;
    if (value == "B1") return LanguageLevel::B1;
    if (value == "B2") return LanguageLevel::B2;
    if (value == "C1") return LanguageLevel::C1;
    if (value == "C2") return LanguageLevel::C2;

    return LanguageLevel::A1;
}

CharacterType StorageManager::characterTypeFromString(const QString& value)
{
    if (value == "male") return CharacterType::Male;
    if (value == "female") return CharacterType::Female;

    return CharacterType::Mixed;
}
