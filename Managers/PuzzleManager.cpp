#include "PuzzleManager.h"

#include <QRandomGenerator>

PuzzleManager::PuzzleManager(QSharedPointer<StorageManager> storageManager) : m_storageManager(storageManager) {}

#include <QRandomGenerator>
#include <QDateTime>
#include <algorithm>

GeneratedPuzzle PuzzleManager::generatePuzzle(
    QVector<int> usedWordsIndexes,
    QVector<int> unlockedImages,
    LanguageLevel level,
    CharacterType characterType
    ) {
    GeneratedPuzzle puzzle;
    puzzle.puzzleId = QString::number(QDateTime::currentMSecsSinceEpoch());

    constexpr int rows = 7;
    constexpr int columns = 7;
    constexpr int targetWordsCount = 6;

    puzzle.rows = rows;
    puzzle.columns = columns;

    auto levelToInt = [](LanguageLevel level) {
        return static_cast<int>(level);
    };

    auto intToLevel = [](int value) {
        return static_cast<LanguageLevel>(value);
    };

    auto takeRandomWord = [](QVector<WordEntry>& source) -> WordEntry {
        const int index = QRandomGenerator::global()->bounded(source.size());
        WordEntry word = source[index];
        source.removeAt(index);
        return word;
    };

    const int currentLevel = levelToInt(level);

    QVector<WordEntry> lowerWords;
    QVector<WordEntry> mainWords;
    QVector<WordEntry> upperWords;

    if (currentLevel > levelToInt(LanguageLevel::A1)) {
        lowerWords = m_storageManager->loadWordsByLevel(
            intToLevel(currentLevel - 1)
            );
    }

    mainWords = m_storageManager->loadWordsByLevel(level);

    if (currentLevel < levelToInt(LanguageLevel::C2)) {
        upperWords = m_storageManager->loadWordsByLevel(
            intToLevel(currentLevel + 1)
            );
    }

    auto removeUsedWords = [&usedWordsIndexes](QVector<WordEntry>& words) {
        words.erase(
            std::remove_if(
                words.begin(),
                words.end(),
                [&usedWordsIndexes](const WordEntry& word) {
                    return usedWordsIndexes.contains(word.id);
                }
                ),
            words.end()
            );
    };

    removeUsedWords(lowerWords);
    removeUsedWords(mainWords);
    removeUsedWords(upperWords);

    QVector<WordEntry> selectedWords;

    if (!lowerWords.isEmpty()) {
        selectedWords.append(takeRandomWord(lowerWords));
    }

    if (!upperWords.isEmpty()) {
        selectedWords.append(takeRandomWord(upperWords));
    }

    while (selectedWords.size() < targetWordsCount && !mainWords.isEmpty()) {
        selectedWords.append(takeRandomWord(mainWords));
    }

    while (selectedWords.size() < targetWordsCount && !lowerWords.isEmpty()) {
        selectedWords.append(takeRandomWord(lowerWords));
    }

    while (selectedWords.size() < targetWordsCount && !upperWords.isEmpty()) {
        selectedWords.append(takeRandomWord(upperWords));
    }

    puzzle.cells.reserve(rows * columns);

    for (int i = 0; i < rows * columns; ++i) {
        PuzzleCell cell;
        cell.index = i;
        cell.row = i / columns;
        cell.column = i % columns;
        puzzle.cells.append(cell);
    }

    auto cellIndex = [columns](int row, int column) {
        return row * columns + column;
    };

    auto canPlaceWord = [&](const QString& word, int row, int column, bool horizontal) {
        if (horizontal && column + word.length() > columns)
            return false;

        if (!horizontal && row + word.length() > rows)
            return false;

        for (int i = 0; i < word.length(); ++i) {
            const int r = horizontal ? row : row + i;
            const int c = horizontal ? column + i : column;
            const PuzzleCell& cell = puzzle.cells[cellIndex(r, c)];

            if (cell.active && cell.letter != word[i].toUpper())
                return false;
        }

        return true;
    };

    auto placeWord = [&](const WordEntry& entry, int wordId, int row, int column, bool horizontal) {
        PuzzleWord puzzleWord;
        puzzleWord.id = entry.id;
        puzzleWord.word = entry.word.toUpper();
        puzzleWord.startRow = row;
        puzzleWord.startColumn = column;
        puzzleWord.direction = horizontal ? "horizontal" : "vertical";

        for (int i = 0; i < puzzleWord.word.length(); ++i) {
            const int r = horizontal ? row : row + i;
            const int c = horizontal ? column + i : column;
            const int index = cellIndex(r, c);

            PuzzleCell& cell = puzzle.cells[index];
            cell.active = true;
            cell.letter = puzzleWord.word[i];

            if (!cell.wordIds.contains(wordId))
                cell.wordIds.append(wordId);

            puzzleWord.cellIndexes.append(index);
        }

        puzzle.words.append(puzzleWord);
    };

    int placedWordCounter = 0;

    for (const WordEntry& entry : selectedWords) {
        const QString word = entry.word.toUpper();

        if (word.length() > columns && word.length() > rows)
            continue;

        bool placed = false;

        for (int attempt = 0; attempt < 80 && !placed; ++attempt) {
            const bool horizontal = QRandomGenerator::global()->bounded(2) == 0;
            const int row = QRandomGenerator::global()->bounded(rows);
            const int column = QRandomGenerator::global()->bounded(columns);

            if (canPlaceWord(word, row, column, horizontal)) {
                placeWord(entry, placedWordCounter, row, column, horizontal);
                placed = true;
                placedWordCounter++;
            }
        }
    }

    QString uniqueLetters;

    for (const PuzzleWord& word : puzzle.words) {
        for (const QChar& ch : word.word) {
            if (!uniqueLetters.contains(ch))
                uniqueLetters.append(ch);
        }
    }

    for (const QChar& ch : uniqueLetters) {
        puzzle.letters.append(QString(ch));
    }

    QVector<ImageEntry> images =
        m_storageManager->loadImagesByPreference(characterType);

    images.erase(
        std::remove_if(
            images.begin(),
            images.end(),
            [&unlockedImages](const ImageEntry& image) {
                return unlockedImages.contains(image.id);
            }
            ),
        images.end()
        );

    if (!images.isEmpty()) {
        const int imageIndex =
            QRandomGenerator::global()->bounded(images.size());

        puzzle.imageId = images[imageIndex].id;
        puzzle.imageSource = images[imageIndex].source;
    }

    return puzzle;
}
