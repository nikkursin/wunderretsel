#include "AppStateManager.h"

#include <QDebug>
#include <QSet>

// ─── Constructor & init ───────────────────────────────────────────────────────

AppStateManager::AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent)
    : QObject(parent), m_storageManager(storageManager), m_puzzleManager(storageManager)
{
}

bool AppStateManager::init()
{
    m_userData = m_storageManager->loadUserData();
    refreshGalleryImages();
    navigateTo(m_userData.isOnboardingCompleted ? Home : Onboarding);
    return true;
}

// ─── Screen navigation ────────────────────────────────────────────────────────

AppStateManager::Screen AppStateManager::currentScreen() const
{
    return m_currentScreen;
}

void AppStateManager::navigateTo(const Screen &screen)
{
    if (m_currentScreen == screen)
        return;

    const Screen leaving = m_currentScreen;
    if (leaving != Play)
        m_history.append(leaving);

    m_currentScreen = screen;
    emit currentScreenChanged();

    if (leaving == Play)
        clearActivePuzzle();
}

void AppStateManager::goHome()
{
    const Screen leaving = m_currentScreen;
    m_history.clear();
    m_currentScreen = Home;
    emit currentScreenChanged();

    if (leaving == Play)
        clearActivePuzzle();
}

void AppStateManager::goBack()
{
    const Screen leaving = m_currentScreen;
    m_currentScreen = m_history.isEmpty() ? Home : m_history.takeLast();
    emit currentScreenChanged();

    if (leaving == Play)
        clearActivePuzzle();
}

void AppStateManager::goPlay()
{
    generateNewPuzzle();
    if (!hasOngoingPuzzle)
    {
        qWarning() << "[AppStateManager] Could not generate a valid puzzle (min 3 words).";
        return;
    }
    navigateTo(Play);
}

void AppStateManager::goGallery()
{
    navigateTo(Gallery);
}
void AppStateManager::goSettings()
{
    navigateTo(Settings);
}
void AppStateManager::goOnboarding()
{
    navigateTo(Onboarding);
}

// ─── Onboarding & preferences ─────────────────────────────────────────────────

void AppStateManager::completeOnboarding(const QString &languageLevel, const QString &characterType)
{
    if (languageLevel.isEmpty() || characterType.isEmpty())
        return;

    m_userData.level = UserData::languageLevelFromString(languageLevel);
    m_userData.characterType = UserData::characterTypeFromString(characterType);
    m_userData.isOnboardingCompleted = true;

    m_storageManager->saveUser(m_userData);
    refreshGalleryImages();
    navigateTo(Home);
}

void AppStateManager::savePreferences()
{
    m_storageManager->saveUser(m_userData);
}

// ─── Language level & character type ─────────────────────────────────────────

QString AppStateManager::languageLevel() const
{
    return UserData::languageLevelToString(m_userData.level);
}

void AppStateManager::setLanguageLevel(const QString &level)
{
    const auto newLevel = UserData::languageLevelFromString(level);
    if (m_userData.level == newLevel)
        return;
    m_userData.level = newLevel;
    emit languageLevelChanged();
}

QString AppStateManager::characterType() const
{
    return UserData::characterTypeToString(m_userData.characterType);
}

void AppStateManager::setCharacterType(const QString &type)
{
    const auto newType = UserData::characterTypeFromString(type);
    if (m_userData.characterType == newType)
        return;
    m_userData.characterType = newType;
    refreshGalleryImages();
    emit characterTypeChanged();
}

// ─── Theme ────────────────────────────────────────────────────────────────────

// Each theme property maps CharacterType → color string. The Male/Mixed/Female
// ordering is consistent across all helpers; Female is the fallthrough default.

QString AppStateManager::themeTintLight() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#eef7ff";
    case CharacterType::Mixed:
        return "#fffcee";
    case CharacterType::Female:
        return "#fff7fa";
    }
    return "#fff7fa";
}

QString AppStateManager::themeTintMid() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#dcecff";
    case CharacterType::Mixed:
        return "#f8efd5";
    case CharacterType::Female:
        return "#f8dce8";
    }
    return "#f8dce8";
}

QString AppStateManager::themeTintDeep() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#adcbef";
    case CharacterType::Mixed:
        return "#e8d29f";
    case CharacterType::Female:
        return "#e9adc5";
    }
    return "#e9adc5";
}

QString AppStateManager::themeAccentStart() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#5ca8eb";
    case CharacterType::Mixed:
        return "#ebc85c";
    case CharacterType::Female:
        return "#eb5c99";
    }
    return "#eb5c99";
}

QString AppStateManager::themeAccentEnd() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#3974ad";
    case CharacterType::Mixed:
        return "#ad8c39";
    case CharacterType::Female:
        return "#ad3974";
    }
    return "#ad3974";
}

QString AppStateManager::themeAccentSoftStart() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#f4f9ff";
    case CharacterType::Mixed:
        return "#fffdf4";
    case CharacterType::Female:
        return "#fff4f9";
    }
    return "#fff4f9";
}

QString AppStateManager::themeAccentSoftEnd() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#8abcf5";
    case CharacterType::Mixed:
        return "#f5da8a";
    case CharacterType::Female:
        return "#f58ab6";
    }
    return "#f58ab6";
}

QString AppStateManager::themeTextPrimary() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#112335";
    case CharacterType::Mixed:
        return "#352c11";
    case CharacterType::Female:
        return "#35111f";
    }
    return "#35111f";
}

QString AppStateManager::themeTextSecondary() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#3a556b";
    case CharacterType::Mixed:
        return "#6b5a3a";
    case CharacterType::Female:
        return "#6b3a4f";
    }
    return "#6b3a4f";
}

QString AppStateManager::themeTextStrong() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#142a40";
    case CharacterType::Mixed:
        return "#403514";
    case CharacterType::Female:
        return "#401425";
    }
    return "#401425";
}

QString AppStateManager::themeTileBase() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#dde9f4";
    case CharacterType::Mixed:
        return "#f4efdd";
    case CharacterType::Female:
        return "#f4dde6";
    }
    return "#f4dde6";
}

QString AppStateManager::themeTileVeil() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#384682dc";
    case CharacterType::Mixed:
        return "#38dcb446";
    case CharacterType::Female:
        return "#38dc4682";
    }
    return "#38dc4682";
}

QString AppStateManager::themeTileBorder() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#1a19385b";
    case CharacterType::Mixed:
        return "#1a5b4a19";
    case CharacterType::Female:
        return "#1a5b1938";
    }
    return "#1a5b1938";
}

QString AppStateManager::themeControlBorder() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#382f619f";
    case CharacterType::Mixed:
        return "#389f7f2f";
    case CharacterType::Female:
        return "#389f2f61";
    }
    return "#389f2f61";
}

QString AppStateManager::themeControlText() const
{
    switch (m_userData.characterType)
    {
    case CharacterType::Male:
        return "#2f619f";
    case CharacterType::Mixed:
        return "#9f7f2f";
    case CharacterType::Female:
        return "#9f2f61";
    }
    return "#9f2f61";
}

// ─── Puzzle ───────────────────────────────────────────────────────────────────

QVariantMap AppStateManager::currentPuzzle() const
{
    const int rows = m_currentPuzzle.rows > 0 ? m_currentPuzzle.rows : 7;
    const int cols = m_currentPuzzle.columns > 0 ? m_currentPuzzle.columns : 7;
    const int cellCount = rows * cols;

    QStringList grid(cellCount);
    QVariantList cellWordIds;
    cellWordIds.reserve(cellCount);

    const QVariant emptyIds = QVariant::fromValue(QVariantList());
    for (int i = 0; i < cellCount; ++i)
        cellWordIds.append(emptyIds);

    for (const PuzzleCell &cell : m_currentPuzzle.cells)
    {
        if (cell.index < 0 || cell.index >= cellCount)
            continue;

        if (cell.active)
            grid[cell.index] = QString(cell.letter);

        QVariantList ids;
        ids.reserve(cell.wordIds.size());
        for (int id : cell.wordIds)
            ids.append(id);
        cellWordIds[cell.index] = QVariant::fromValue(ids);
    }

    QStringList words;
    QVariantList wordRows;
    words.reserve(m_currentPuzzle.words.size());
    wordRows.reserve(m_currentPuzzle.words.size());

    for (const PlacedWord &word : m_currentPuzzle.words)
    {
        words.append(word.word);
        wordRows.append(word.row);
    }

    return {
        {"rows", rows},
        {"columns", cols},
        {"imageSource", m_currentPuzzle.imageSource},
        {"letters", m_currentPuzzle.letters},
        {"words", words},
        {"wordRows", wordRows},
        {"cellWordIds", cellWordIds},
        {"grid", grid},
    };
}

bool AppStateManager::ongoingPuzzlePresent() const
{
    return hasOngoingPuzzle && m_currentPuzzle.solvedWordIds.size() < m_currentPuzzle.words.size();
}

void AppStateManager::generateNewPuzzle()
{
    constexpr int kMinPlayableWords = 3;
    constexpr int kMaxRetries = 32;

    const QSet<int> unlockedImageIds(m_userData.unlockedImagesIds.begin(),
                                     m_userData.unlockedImagesIds.end());

    auto tryGenerate = [&](const QSet<int> &excludedWordIds)
    {
        return m_puzzleManager.generatePuzzle(excludedWordIds, unlockedImageIds, m_userData.level,
                                              m_userData.characterType,
                                              m_userData.solvedPuzzleCount);
    };

    GeneratedPuzzle nextPuzzle;
    bool generated = false;

    for (int retry = 0; retry < kMaxRetries && !generated; ++retry)
    {
        const QSet<int> usedWordIds(m_userData.usedWordsIds.begin(), m_userData.usedWordsIds.end());

        GeneratedPuzzle candidate = tryGenerate(usedWordIds);
        if (candidate.words.size() >= kMinPlayableWords)
        {
            nextPuzzle = std::move(candidate);
            generated = true;
            break;
        }

        candidate = tryGenerate({});
        if (candidate.words.size() >= kMinPlayableWords)
        {
            m_userData.usedWordsIds.clear();
            nextPuzzle = std::move(candidate);
            generated = true;
            break;
        }

        qWarning() << "[AppStateManager] Generation retry" << (retry + 1)
                   << "produced invalid puzzle; trying again.";
    }

    if (!generated)
    {
        qCritical() << "[AppStateManager] Could not generate a valid puzzle after" << kMaxRetries
                    << "retries (min" << kMinPlayableWords << "words).";
        hasOngoingPuzzle = false;
        m_currentPuzzle = GeneratedPuzzle{};
        emit currentPuzzleChanged();
        return;
    }

    m_currentPuzzle = std::move(nextPuzzle);
    hasOngoingPuzzle = true;

    for (const PlacedWord &word : m_currentPuzzle.words)
    {
        if (word.id >= 0 && !m_userData.usedWordsIds.contains(word.id))
            m_userData.usedWordsIds.append(word.id);
    }

    m_storageManager->saveUser(m_userData);
    emit currentPuzzleChanged();
}

void AppStateManager::notifyPuzzleSolved()
{
    if (!hasOngoingPuzzle)
        return;

    hasOngoingPuzzle = false;
    m_userData.solvedPuzzleCount += 1;

    const bool unlockedNew = m_currentPuzzle.imageId >= 0 &&
                             !m_userData.unlockedImagesIds.contains(m_currentPuzzle.imageId);

    if (unlockedNew)
        m_userData.unlockedImagesIds.append(m_currentPuzzle.imageId);

    m_storageManager->saveUser(m_userData);

    if (unlockedNew)
        emit galleryImagesChanged();
}

void AppStateManager::clearActivePuzzle()
{
    if (m_currentPuzzle.words.isEmpty() && !hasOngoingPuzzle)
        return;
    m_currentPuzzle = GeneratedPuzzle{};
    hasOngoingPuzzle = false;
    emit currentPuzzleChanged();
}

// ─── Gallery ──────────────────────────────────────────────────────────────────

QVariantList AppStateManager::galleryImages() const
{
    const QSet<int> unlocked(m_userData.unlockedImagesIds.begin(),
                             m_userData.unlockedImagesIds.end());

    QVariantList list;
    list.reserve(m_galleryImages.size());

    for (const ImageEntry &entry : m_galleryImages)
    {
        list.append(QVariantMap{
            {"id", entry.id},
            {"source", entry.source},
            {"unlocked", unlocked.contains(entry.id)},
        });
    }

    return list;
}

int AppStateManager::unlockedImagesCount() const
{
    const QSet<int> unlocked(m_userData.unlockedImagesIds.begin(),
                             m_userData.unlockedImagesIds.end());
    int count = 0;
    for (const ImageEntry &entry : m_galleryImages)
        if (unlocked.contains(entry.id))
            ++count;
    return count;
}

int AppStateManager::totalImagesCount() const
{
    return m_galleryImages.size();
}

void AppStateManager::refreshGalleryImages()
{
    m_galleryImages = m_storageManager->loadImagesByPreference(m_userData.characterType);
    emit galleryImagesChanged();
}