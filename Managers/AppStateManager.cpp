#include "StorageManager.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>

// ─── Constructor ──────────────────────────────────────────────────────────────

StorageManager::StorageManager()
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_userFilePath = dataDir + "/user.json";

    if (QFile::exists(m_userFilePath))
        return;

    if (saveUser(UserData{}))
        qInfo() << "Created default user.json at" << m_userFilePath;
    else
        qWarning() << "Failed to create default user.json at" << m_userFilePath;
}

// ─── User data ────────────────────────────────────────────────────────────────

UserData StorageManager::loadUserData()
{
    UserData data;
    qInfo() << "Loading user data from" << m_userFilePath;

    QFile file(m_userFilePath);

    if (!file.exists())
    {
        qWarning() << "user.json does not exist:" << m_userFilePath;
        return data;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open user.json for reading:" << m_userFilePath
                   << "error:" << file.errorString();
        return data;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "Invalid user.json:" << m_userFilePath
                   << "parseError:" << parseError.errorString();
        return data;
    }

    const QJsonObject root = doc.object();
    const QJsonObject preferences = root.value("preferences").toObject();
    const QJsonObject progress = root.value("progress").toObject();

    data.isOnboardingCompleted = root.value("onboardingCompleted").toBool(false);
    data.level =
        UserData::languageLevelFromString(preferences.value("languageLevel").toString("A1"));
    data.characterType =
        UserData::characterTypeFromString(preferences.value("characterType").toString("mixed"));
    data.solvedPuzzleCount = progress.value("solvedPuzzleCount").toInt(0);

    auto collectIds = [](const QJsonArray &arr)
    {
        QList<int> ids;
        for (const QJsonValue &v : arr)
            if (v.isDouble())
                ids.append(v.toInt());
        return ids;
    };

    data.usedWordsIds = collectIds(progress.value("usedWordIds").toArray());
    data.unlockedImagesIds = collectIds(progress.value("unlockedImageIds").toArray());

    return data;
}

bool StorageManager::saveUser(const UserData &userData)
{
    QJsonArray usedWordsArray;
    for (int id : userData.usedWordsIds)
        usedWordsArray.append(id);

    QJsonArray unlockedImagesArray;
    for (int id : userData.unlockedImagesIds)
        unlockedImagesArray.append(id);

    QJsonObject progress;
    progress["usedWordIds"] = usedWordsArray;
    progress["unlockedImageIds"] = unlockedImagesArray;
    progress["solvedPuzzleCount"] = userData.solvedPuzzleCount;

    QJsonObject preferences;
    preferences["languageLevel"] = UserData::languageLevelToString(userData.level);
    preferences["characterType"] = UserData::characterTypeToString(userData.characterType);

    QJsonObject root;
    root["onboardingCompleted"] = userData.isOnboardingCompleted;
    root["preferences"] = preferences;
    root["progress"] = progress;

    const QByteArray jsonData = QJsonDocument(root).toJson(QJsonDocument::Indented);

    QFile file(m_userFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "Failed to open user.json for writing:" << m_userFilePath
                   << "error:" << file.errorString();
        return false;
    }

    if (file.write(jsonData) == -1)
    {
        qWarning() << "Failed to write user.json:" << m_userFilePath
                   << "error:" << file.errorString();
        return false;
    }

    qInfo() << "Saved user.json to" << m_userFilePath;
    return true;
}

// ─── Asset loaders ────────────────────────────────────────────────────────────

QVector<WordEntry> StorageManager::loadWordsByLevel(LanguageLevel level)
{
    QVector<WordEntry> result;

    const QString levelKey = UserData::languageLevelToString(level);
    const QString wordsPath = QStringLiteral(":/assets/data/words.json");

    QFile file(wordsPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open words.json:" << wordsPath << "error:" << file.errorString();
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (!doc.isObject())
    {
        qWarning() << "Invalid words.json, expected object:" << wordsPath
                   << "parseError:" << parseError.errorString();
        return result;
    }

    for (const QJsonValue &value : doc.object().value(levelKey).toArray())
    {
        if (!value.isObject())
            continue;

        const QJsonObject obj = value.toObject();
        const int id = obj.value("id").toInt(-1);
        const QString word = obj.value("word").toString();

        if (id == -1 || word.isEmpty())
            continue;

        result.append({id, word, level});
    }

    return result;
}

QVector<ImageEntry> StorageManager::loadImagesByPreference(CharacterType characterType)
{
    QVector<ImageEntry> result;

    const QString imagesPath = QStringLiteral(":/assets/data/images.json");
    QFile file(imagesPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open images.json:" << imagesPath << "error:" << file.errorString();
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (!doc.isArray())
    {
        qWarning() << "Invalid images.json, expected array:" << imagesPath
                   << "parseError:" << parseError.errorString();
        return result;
    }

    const QString selectedType = UserData::characterTypeToString(characterType);

    auto normalizeQrcPath = [](const QString &raw) -> QString
    {
        if (raw.startsWith(QStringLiteral("qrc:/")))
            return raw;
        if (raw.startsWith(QLatin1Char(':')))
            return QStringLiteral("qrc") + raw;
        if (!raw.isEmpty() && !raw.contains(QStringLiteral("://")))
        {
            const QString withSlash =
                raw.startsWith(QLatin1Char('/')) ? raw : QLatin1Char('/') + raw;
            return QStringLiteral("qrc") + withSlash;
        }
        return raw;
    };

    for (const QJsonValue &value : doc.array())
    {
        if (!value.isObject())
            continue;

        const QJsonObject obj = value.toObject();
        const QString imgType = obj.value("characterType").toString();

        if (characterType != CharacterType::Mixed && imgType != selectedType)
            continue;

        const int id = obj.value("id").toInt(-1);
        const QString source = normalizeQrcPath(obj.value("source").toString().trimmed());

        if (id == -1 || source.isEmpty())
            continue;

        result.append({id, source, UserData::characterTypeFromString(imgType)});
    }

    return result;
}
