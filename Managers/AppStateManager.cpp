#include "AppStateManager.h"
#include <QDebug>

AppStateManager::AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent) : QObject{parent}, m_storageManager(storageManager), m_puzzleManager(storageManager){}

bool AppStateManager::init() {
    m_userData = m_storageManager->loadUserData();

    if(m_userData.isOnboardingCompleted) navigateTo(Home);
        else navigateTo(Onboarding);


    return true;
}

AppStateManager::Screen AppStateManager::currentScreen() const
{
    return m_currentScreen;
}

bool AppStateManager::onboardingCompleted() const {
    return m_userData.isOnboardingCompleted;
}

QString AppStateManager::languageLevel() const
{
    return UserData::languageLevelToString(m_userData.level);
}

void AppStateManager::setLanguageLevel(const QString& level)
{
    auto newLevel = UserData::languageLevelFromString(level);

    if (m_userData.level == newLevel)
        return;

    m_userData.level = newLevel;
    emit languageLevelChanged();
}

QString AppStateManager::characterType() const
{
    return UserData::characterTypeToString(m_userData.characterType);
}

void AppStateManager::setCharacterType(const QString& type)
{
    auto newType = UserData::characterTypeFromString(type);

    if (m_userData.characterType == newType)
        return;

    m_userData.characterType = newType;
    emit characterTypeChanged();
}

QVariantMap AppStateManager::currentPuzzle() const
{
    // Flatten the GeneratedPuzzle into the QVariantMap shape WRPlayScreen.qml
    // expects. With real crosswords a cell can belong to multiple words,
    // so we expose `cellWordIds` (list-per-cell of word indexes) and let
    // QML decide which cells to reveal.
    //
    //   - rows / columns : grid dimensions (drive QML layout)
    //   - words          : QStringList of canonical words
    //   - letters        : QStringList of unique letters for the wheel
    //   - grid           : QStringList of rows*columns entries; "" = gap,
    //                      otherwise the uppercase letter of the cell
    //   - cellWordIds    : QVariantList of QVariantLists; per-cell list of
    //                      word indexes into `words` that cover this cell
    //   - wordRows       : kept for backward-compatibility (start row of
    //                      each word)
    //   - imageSource    : qrc path of the reward image
    QVariantMap puzzle;

    const int rows = m_currentPuzzle.rows > 0 ? m_currentPuzzle.rows : 7;
    const int columns = m_currentPuzzle.columns > 0 ? m_currentPuzzle.columns : 7;
    const int cellCount = rows * columns;

    QStringList grid;
    grid.reserve(cellCount);
    for (int i = 0; i < cellCount; ++i)
        grid.append(QString());

    // NB: `QList<QVariant>::append(const QList<QVariant>&)` would *concatenate*,
    // not insert, if we passed a bare QVariantList. Wrap in QVariant explicitly
    // and pre-size the list so [] is always a valid in-place write.
    QVariantList cellWordIds;
    cellWordIds.reserve(cellCount);
    const QVariant emptyIds = QVariant::fromValue(QVariantList());
    for (int i = 0; i < cellCount; ++i)
        cellWordIds.append(emptyIds);

    for (const PuzzleCell& cell : m_currentPuzzle.cells) {
        if (cell.index < 0 || cell.index >= cellCount)
            continue;
        if (cell.active)
            grid[cell.index] = QString(cell.letter);

        QVariantList ids;
        ids.reserve(cell.wordIds.size());
        for (int id : cell.wordIds)
            ids.append(QVariant(id));
        cellWordIds[cell.index] = QVariant::fromValue(ids);
    }

    QStringList words;
    QVariantList wordRows;
    words.reserve(m_currentPuzzle.words.size());
    wordRows.reserve(m_currentPuzzle.words.size());

    for (const PlacedWord& word : m_currentPuzzle.words) {
        words.append(word.word);
        wordRows.append(word.row);
    }

    puzzle["rows"] = rows;
    puzzle["columns"] = columns;
    puzzle["imageSource"] = m_currentPuzzle.imageSource;
    puzzle["letters"] = m_currentPuzzle.letters;
    puzzle["words"] = words;
    puzzle["wordRows"] = wordRows;
    puzzle["cellWordIds"] = cellWordIds;
    puzzle["grid"] = grid;

    return puzzle;
}

void AppStateManager::generateNewPuzzle()
{
    m_userData = m_storageManager->loadUserData();

    const QSet<int> usedWordIds(m_userData.usedWordsIds.begin(),
                                 m_userData.usedWordsIds.end());
    const QSet<int> unlockedImageIds(m_userData.unlockedImagesIds.begin(),
                                      m_userData.unlockedImagesIds.end());

    m_currentPuzzle = m_puzzleManager.generatePuzzle(
        usedWordIds,
        unlockedImageIds,
        m_userData.level,
        m_userData.characterType,
        m_userData.solvedPuzzleCount
        );

    hasOngoingPuzzle = !m_currentPuzzle.words.isEmpty();

    for (const PlacedWord& word : m_currentPuzzle.words) {
        if (word.id < 0) continue;
        if (!m_userData.usedWordsIds.contains(word.id))
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

    if (m_currentPuzzle.imageId >= 0
        && !m_userData.unlockedImagesIds.contains(m_currentPuzzle.imageId)) {
        m_userData.unlockedImagesIds.append(m_currentPuzzle.imageId);
    }

    m_storageManager->saveUser(m_userData);
}

void AppStateManager::goHome()
{
    m_history.clear();
    m_currentScreen = Home;
    emit currentScreenChanged();
}

void AppStateManager::goPlay()
{
    // TODO: Add check on hasOngoingPuzzle
    generateNewPuzzle();
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

void AppStateManager::goBack()
{
    if (m_history.isEmpty()) {
        goHome();
        return;
    }

    m_currentScreen = m_history.takeLast();
    emit currentScreenChanged();
}

void AppStateManager::completeOnboarding(const QString &languageLevel,
                                         const QString &characterType)
{
    if (languageLevel.isEmpty() || characterType.isEmpty())
        return;

    m_userData.level = UserData::languageLevelFromString(languageLevel);
    m_userData.characterType = UserData::characterTypeFromString(characterType);
    m_userData.isOnboardingCompleted = true;

    m_storageManager->saveUser(m_userData);

    navigateTo(Home);
}

void AppStateManager::savePreferences()
{
    m_storageManager->saveUser(m_userData);
}

void AppStateManager::navigateTo(const Screen& screen)
{
    if (m_currentScreen == screen)
        return;

    m_history.append(m_currentScreen);
    m_currentScreen = screen;

    emit currentScreenChanged();
}

bool AppStateManager::ongoingPuzzlePresent() const
{
    return hasOngoingPuzzle &&
           m_currentPuzzle.solvedWordIds.size() < m_currentPuzzle.words.size();
}
