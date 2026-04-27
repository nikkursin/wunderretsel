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
        UserData::languageLevelFromString(
            preferences.value("languageLevel").toString("A1")
            );

    data.characterType =
        UserData::characterTypeFromString(
            preferences.value("characterType").toString("mixed")
            );

    return data;
}

bool StorageManager::saveUser(const UserData& userData) {
    QJsonObject root;

           // onboarding flag
    root["onboardingCompleted"] = userData.isOnboardingCompleted;

           // preferences object
    QJsonObject preferences;

    preferences["languageLevel"] =
        UserData::languageLevelToString(userData.level);

    preferences["characterType"] =
        UserData::characterTypeToString(userData.characterType);

    root["preferences"] = preferences;

           // create JSON document
    QJsonDocument doc(root);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    QFile file(m_userFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for writing:" << m_userFilePath;
        return false;
    }

    if (file.write(jsonData) == -1) {
        qDebug() << "Failed to write JSON to file";
        file.close();
        return false;
    }

    file.close();

    return true;
}

