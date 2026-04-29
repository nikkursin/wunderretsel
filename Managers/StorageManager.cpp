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

    // Ensure user.json exists on every app launch so subsequent reads/writes
    // always operate on a real file.
    if (!QFile::exists(m_userFilePath)) {
        if (!saveUser(UserData{})) {
            qWarning() << "Failed to create default user.json at"
                       << m_userFilePath;
        } else {
            qInfo() << "Created default user.json at"
                    << m_userFilePath;
        }
    }
}

UserData StorageManager::loadUserData()
{
    UserData data;

    qInfo() << "Loading user data from" << m_userFilePath;

    QFile file(m_userFilePath);

    if (!file.exists()) {
        qWarning() << "user.json does not exist:"
                   << m_userFilePath;
        return data;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open user.json for reading:"
                   << m_userFilePath
                   << "error:" << file.errorString();
        return data;
    }

    const QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc =
        QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid user.json:"
                   << m_userFilePath
                   << "parseError:" << parseError.errorString();
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

    const QJsonObject progress =
        root.value("progress").toObject();

    const QJsonArray usedWordsArray =
        progress.value("usedWordIds").toArray();

    for (const QJsonValue& value : usedWordsArray) {
        if (value.isDouble()) {
            data.usedWordsIds.append(value.toInt());
        }
    }

    const QJsonArray unlockedImagesArray =
        progress.value("unlockedImageIds").toArray();

    for (const QJsonValue& value : unlockedImagesArray) {
        if (value.isDouble()) {
            data.unlockedImagesIds.append(value.toInt());
        }
    }

    data.solvedPuzzleCount =
        progress.value("solvedPuzzleCount").toInt(0);

    return data;
}

bool StorageManager::saveUser(const UserData& userData)
{
    QJsonObject root;

    root["onboardingCompleted"] = userData.isOnboardingCompleted;

    QJsonObject preferences;

    preferences["languageLevel"] =
        UserData::languageLevelToString(userData.level);

    preferences["characterType"] =
        UserData::characterTypeToString(userData.characterType);

    root["preferences"] = preferences;

    QJsonObject progress;

    QJsonArray usedWordsArray;

    for (int wordId : userData.usedWordsIds) {
        usedWordsArray.append(wordId);
    }

    progress["usedWordIds"] = usedWordsArray;

    QJsonArray unlockedImagesArray;

    for (int imageId : userData.unlockedImagesIds) {
        unlockedImagesArray.append(imageId);
    }

    progress["unlockedImageIds"] = unlockedImagesArray;

    progress["solvedPuzzleCount"] = userData.solvedPuzzleCount;

    root["progress"] = progress;

    QJsonDocument doc(root);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    QFile file(m_userFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to open user.json for writing:"
                   << m_userFilePath
                   << "error:" << file.errorString();
        return false;
    }

    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write user.json:"
                   << m_userFilePath
                   << "error:" << file.errorString();
        file.close();
        return false;
    }

    file.close();

    qInfo() << "Saved user.json to" << m_userFilePath;
    return true;
}

QVector<WordEntry> StorageManager::loadWordsByLevel(LanguageLevel level)
{
    QVector<WordEntry> result;

    const QString levelKey = UserData::languageLevelToString(level);
    const QString wordsPath = QStringLiteral(":/assets/data/words.json");

    QFile file(wordsPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open words.json:"
                   << wordsPath
                   << "error:" << file.errorString();
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (!doc.isObject()) {
        qWarning() << "Invalid words.json root, expected object:"
                   << wordsPath
                   << "parseError:" << parseError.errorString();
        return result;
    }

    const QJsonArray wordsArray = doc.object().value(levelKey).toArray();

    for (const QJsonValue& value : wordsArray) {
        if (!value.isObject())
            continue;

        const QJsonObject obj = value.toObject();

        WordEntry entry;
        entry.id = obj.value("id").toInt(-1);
        entry.word = obj.value("word").toString();
        entry.level = level;

        if (entry.id != -1 && !entry.word.isEmpty())
            result.append(entry);
    }

    return result;
}

QVector<ImageEntry> StorageManager::loadImagesByPreference(CharacterType characterType)
{
    QVector<ImageEntry> result;

    const QString imagesPath = QStringLiteral(":/assets/data/images.json");
    QFile file(imagesPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open images.json:"
                   << imagesPath
                   << "error:" << file.errorString();
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (!doc.isArray()) {
        qWarning() << "Invalid images.json root, expected array:"
                   << imagesPath
                   << "parseError:" << parseError.errorString();
        return result;
    }

    const QString selectedType = UserData::characterTypeToString(characterType);

    for (const QJsonValue& value : doc.array()) {
        if (!value.isObject())
            continue;

        const QJsonObject obj = value.toObject();

        const QString imageType = obj.value("characterType").toString();

        if (!(characterType == CharacterType::Mixed ||
              imageType == selectedType))
            continue;

        ImageEntry entry;
        entry.id = obj.value("id").toInt(-1);

        // Normalise the source into a QML-loadable URL. The dictionary
        // historically stored Qt resource paths in either the C++ form
        // (":/assets/...") or as a bare relative path ("assets/..."); QML's
        // Image::source is a URL and only understands the "qrc:/" scheme.
        // Coerce all of those into a single canonical form here so the QML
        // side never has to think about it.
        QString rawSource = obj.value("source").toString().trimmed();
        if (rawSource.startsWith(QStringLiteral("qrc:/"))) {
            entry.source = rawSource;
        } else if (rawSource.startsWith(QLatin1Char(':'))) {
            entry.source = QStringLiteral("qrc") + rawSource;       // ":/x" -> "qrc:/x"
        } else if (!rawSource.isEmpty()
                   && !rawSource.contains(QStringLiteral("://"))) {
            // bare path like "assets/images/.." -> "qrc:/assets/images/.."
            if (!rawSource.startsWith(QLatin1Char('/')))
                rawSource.prepend(QLatin1Char('/'));
            entry.source = QStringLiteral("qrc") + rawSource;
        } else {
            entry.source = rawSource;                               // file://, http://, ...
        }

        entry.characterType = UserData::characterTypeFromString(imageType);

        if (entry.id != -1 && !entry.source.isEmpty())
            result.append(entry);
    }

    return result;
}
