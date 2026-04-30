#include "PuzzleManager.h"

#include <QChar>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QSet>
#include <QString>
#include <QtGlobal>

#include <algorithm>

namespace
{

constexpr int kGridRows = 7;
constexpr int kGridColumns = 7;
constexpr int kMinWordCount = 3;
constexpr int kMaxWordCount = 6;
constexpr int kMinValidWords = 3;
constexpr int kMinWordLength = 2;
constexpr int kMaxWordLength = 8;
constexpr int kMaxWheelLetters = 8;
constexpr int kMaxAttempts = 200;
constexpr qint64 kTimeBudgetMs = 600;
constexpr int kCrossingBonus = 10;
constexpr int kIsolationPenalty = -1;

enum Direction
{
    Horizontal = 0,
    Vertical = 1
};

struct Step
{
    int row, col;
};

constexpr Step kDirStep[2] = {{0, 1}, {1, 0}};

// ─── Data structures ─────────────────────────────────────────────────────────

struct WorkingGrid
{
    int rows = 0;
    int cols = 0;
    QVector<QVector<QChar>> cells;
};

struct Placement
{
    int row = 0;
    int col = 0;
    Direction dir = Horizontal;
    int score = 0;
    int crossings = 0;
};

struct AttemptResult
{
    QVector<PlacedWord> placed;
    int totalScore = 0;
    int totalCrossings = 0;
    int unplacedCount = 0;
};

struct DifficultyTargets
{
    int wordCount;
    int maxWordLength;
};

// ─── Small utilities ─────────────────────────────────────────────────────────

QString canonicalWord(const QString &raw)
{
    return raw.trimmed().toUpper();
}

template <typename T> void shuffleInPlace(QVector<T> &v)
{
    auto *rng = QRandomGenerator::global();
    for (int i = v.size() - 1; i > 0; --i)
    {
        const int j = rng->bounded(i + 1);
        if (j != i)
            std::swap(v[i], v[j]);
    }
}

inline bool inBounds(const WorkingGrid &g, int r, int c)
{
    return r >= 0 && r < g.rows && c >= 0 && c < g.cols;
}

inline QChar cellAt(const WorkingGrid &g, int r, int c)
{
    return inBounds(g, r, c) ? g.cells[r][c] : QChar();
}

QHash<QChar, int> letterCounts(const QString &word)
{
    QHash<QChar, int> counts;
    for (const QChar &ch : word)
        ++counts[ch];
    return counts;
}

QVector<WordEntry> dedupeByCanonical(QVector<WordEntry> pool)
{
    QSet<QString> seen;
    QVector<WordEntry> result;
    result.reserve(pool.size());
    for (const WordEntry &entry : pool)
    {
        const QString upper = canonicalWord(entry.word);
        if (seen.contains(upper))
            continue;
        seen.insert(upper);
        result.append(entry);
    }
    return result;
}

// ─── Placement logic ─────────────────────────────────────────────────────────

bool tryScorePlacement(const WorkingGrid &g, const QString &word, int row, int col, Direction dir,
                       bool isFirstWord, Placement &out)
{
    const int len = word.length();
    if (len <= 0)
        return false;

    const Step step = kDirStep[dir];
    const Step perp = kDirStep[dir == Horizontal ? Vertical : Horizontal];

    const int endRow = row + step.row * (len - 1);
    const int endCol = col + step.col * (len - 1);
    if (!inBounds(g, row, col) || !inBounds(g, endRow, endCol))
        return false;

    if (!cellAt(g, row - step.row, col - step.col).isNull())
        return false;
    if (!cellAt(g, endRow + step.row, endCol + step.col).isNull())
        return false;

    int crossings = 0;
    for (int i = 0; i < len; ++i)
    {
        const int r = row + step.row * i;
        const int c = col + step.col * i;
        const QChar existing = g.cells[r][c];
        const QChar incoming = word.at(i);

        if (!existing.isNull())
        {
            if (existing != incoming)
                return false;
            ++crossings;
            continue;
        }

        if (!cellAt(g, r + perp.row, c + perp.col).isNull())
            return false;
        if (!cellAt(g, r - perp.row, c - perp.col).isNull())
            return false;
    }

    int score;
    if (crossings > 0)
        score = crossings * kCrossingBonus;
    else if (isFirstWord)
        score = 0;
    else
        score = kIsolationPenalty;

    out = {row, col, dir, score, crossings};
    return true;
}

void stampWord(WorkingGrid &g, const QString &word, const Placement &p)
{
    const Step step = kDirStep[p.dir];
    for (int i = 0; i < word.length(); ++i)
        g.cells[p.row + step.row * i][p.col + step.col * i] = word.at(i);
}

bool findBestPlacement(const WorkingGrid &g, const QString &word, bool isFirstWord, Placement &out)
{
    Placement best;
    bool found = false;

    for (int r = 0; r < g.rows; ++r)
    {
        for (int c = 0; c < g.cols; ++c)
        {
            for (Direction dir : {Horizontal, Vertical})
            {
                Placement candidate;
                if (!tryScorePlacement(g, word, r, c, dir, isFirstWord, candidate))
                    continue;

                if (isFirstWord)
                {
                    const Step step = kDirStep[dir];
                    const int len = word.length();
                    const int midRow = r + step.row * (len / 2);
                    const int midCol = c + step.col * (len / 2);
                    const int dr = midRow - g.rows / 2;
                    const int dc = midCol - g.cols / 2;
                    candidate.score = -(dr * dr + dc * dc);
                }

                if (!found || candidate.score > best.score)
                {
                    best = candidate;
                    found = true;
                }
            }
        }
    }

    if (found)
        out = best;
    return found;
}

// ─── Generation ──────────────────────────────────────────────────────────────

AttemptResult tryGenerateOnce(const QVector<WordEntry> &candidates, int targetWordCount, int rows,
                              int cols)
{
    AttemptResult result;

    WorkingGrid grid;
    grid.rows = rows;
    grid.cols = cols;
    grid.cells.resize(rows);
    for (int r = 0; r < rows; ++r)
        grid.cells[r].fill(QChar(), cols);

    QHash<QChar, int> wheelCounts;
    int wheelTotal = 0;

    QSet<QString> placedWords;
    QSet<int> placedIds;
    int placedCount = 0;

    for (const WordEntry &entry : candidates)
    {
        if (placedCount >= targetWordCount)
            break;

        const QString word = canonicalWord(entry.word);
        const int wordLen = word.length();

        if (wordLen < kMinWordLength || wordLen > kMaxWordLength)
            continue;
        if (wordLen > qMin(rows, cols))
            continue;
        if (placedWords.contains(word))
            continue;
        if (entry.id >= 0 && placedIds.contains(entry.id))
            continue;

        const auto counts = letterCounts(word);
        QHash<QChar, int> projectedWheel = wheelCounts;
        int projectedTotal = wheelTotal;
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
        {
            const int deficit = it.value() - projectedWheel.value(it.key(), 0);
            if (deficit > 0)
            {
                projectedTotal += deficit;
                projectedWheel[it.key()] = it.value();
            }
        }
        if (projectedTotal > kMaxWheelLetters)
        {
            ++result.unplacedCount;
            continue;
        }

        Placement placement;
        if (!findBestPlacement(grid, word, placedCount == 0, placement))
        {
            ++result.unplacedCount;
            continue;
        }

        stampWord(grid, word, placement);

        PlacedWord pw;
        pw.id = entry.id;
        pw.word = word;
        pw.row = placement.row;
        pw.col = placement.col;
        pw.horizontal = (placement.dir == Horizontal);
        result.placed.append(pw);
        result.totalScore += placement.score;
        result.totalCrossings += placement.crossings;

        placedWords.insert(word);
        if (entry.id >= 0)
            placedIds.insert(entry.id);

        wheelCounts = projectedWheel;
        wheelTotal = projectedTotal;
        ++placedCount;
    }

    return result;
}

// ─── Difficulty ───────────────────────────────────────────────────────────────

DifficultyTargets computeDifficulty(int difficultyFactor)
{
    const int df = qMax(0, difficultyFactor);
    return {qBound(kMinWordCount, kMinWordCount + df / 2, kMaxWordCount),
            qBound(kMinWordLength + 2, 4 + df, kMaxWordLength)};
}

// ─── Candidate pool builders ──────────────────────────────────────────────────

QVector<WordEntry> buildCandidatePool(StorageManager &storage, LanguageLevel level,
                                      const QSet<int> &usedWordIds, int maxWordLength)
{
    auto loadFiltered = [&](LanguageLevel lvl)
    {
        QVector<WordEntry> words = storage.loadWordsByLevel(lvl);
        words.erase(std::remove_if(words.begin(), words.end(),
                                   [&](const WordEntry &w)
                                   {
                                       const int len = canonicalWord(w.word).length();
                                       return w.id < 0 || usedWordIds.contains(w.id) ||
                                              len < kMinWordLength || len > maxWordLength;
                                   }),
                    words.end());
        return words;
    };

    const int lvl = static_cast<int>(level);
    QVector<WordEntry> pool = loadFiltered(level);
    if (lvl > static_cast<int>(LanguageLevel::A1))
        pool += loadFiltered(static_cast<LanguageLevel>(lvl - 1));
    if (lvl < static_cast<int>(LanguageLevel::C2))
        pool += loadFiltered(static_cast<LanguageLevel>(lvl + 1));

    return dedupeByCanonical(std::move(pool));
}

QVector<WordEntry> buildCandidatePoolAllLevels(StorageManager &storage, int maxWordLength)
{
    QVector<WordEntry> pool;
    for (int lvl = static_cast<int>(LanguageLevel::A1); lvl <= static_cast<int>(LanguageLevel::C2);
         ++lvl)
    {
        QVector<WordEntry> words = storage.loadWordsByLevel(static_cast<LanguageLevel>(lvl));
        words.erase(std::remove_if(words.begin(), words.end(),
                                   [&](const WordEntry &w)
                                   {
                                       const int len = canonicalWord(w.word).length();
                                       return w.id < 0 || len < kMinWordLength ||
                                              len > maxWordLength;
                                   }),
                    words.end());
        pool += words;
    }
    return dedupeByCanonical(std::move(pool));
}

// ─── Reward image ─────────────────────────────────────────────────────────────

void selectRewardImage(StorageManager &storage, CharacterType characterType,
                       const QSet<int> &unlockedIds, int &outId, QString &outSource)
{
    outId = -1;
    outSource.clear();

    QVector<ImageEntry> images = storage.loadImagesByPreference(characterType);
    images.erase(std::remove_if(images.begin(), images.end(),
                                [&](const ImageEntry &e) { return unlockedIds.contains(e.id); }),
                 images.end());

    if (images.isEmpty())
        return;

    const int idx = QRandomGenerator::global()->bounded(images.size());
    outId = images[idx].id;
    outSource = images[idx].source;
}

// ─── Finalization ─────────────────────────────────────────────────────────────

GeneratedPuzzle finalizePuzzle(const AttemptResult &attempt, int rows, int cols)
{
    GeneratedPuzzle puzzle;
    puzzle.puzzleId = QString::number(QDateTime::currentMSecsSinceEpoch());
    puzzle.rows = rows;
    puzzle.columns = cols;
    puzzle.score = attempt.totalScore;

    puzzle.grid.resize(rows);
    for (int r = 0; r < rows; ++r)
        puzzle.grid[r].fill(QChar(), cols);

    for (const PlacedWord &pw : attempt.placed)
    {
        const Step step = kDirStep[pw.horizontal ? Horizontal : Vertical];
        for (int i = 0; i < pw.word.length(); ++i)
            puzzle.grid[pw.row + step.row * i][pw.col + step.col * i] = pw.word.at(i);
    }

    puzzle.cells.reserve(rows * cols);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            const QChar ch = puzzle.grid[r][c];
            PuzzleCell cell;
            cell.index = r * cols + c;
            cell.row = r;
            cell.column = c;
            cell.active = !ch.isNull();
            cell.letter = ch;
            puzzle.cells.append(cell);
        }
    }

    puzzle.words = attempt.placed;
    for (int wi = 0; wi < puzzle.words.size(); ++wi)
    {
        PlacedWord &pw = puzzle.words[wi];
        const Step step = kDirStep[pw.horizontal ? Horizontal : Vertical];
        pw.cellIndexes.reserve(pw.word.length());
        for (int i = 0; i < pw.word.length(); ++i)
        {
            const int flatIndex = (pw.row + step.row * i) * cols + (pw.col + step.col * i);
            pw.cellIndexes.append(flatIndex);
            PuzzleCell &cell = puzzle.cells[flatIndex];
            if (!cell.wordIds.contains(wi))
                cell.wordIds.append(wi);
        }
    }

    QHash<QChar, int> wheelCounts;
    for (const PlacedWord &pw : puzzle.words)
    {
        const QHash<QChar, int> counts = letterCounts(pw.word);
        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
            wheelCounts[it.key()] = qMax(wheelCounts.value(it.key(), 0), it.value());
    }

    QList<QChar> sortedLetters = wheelCounts.keys();
    std::sort(sortedLetters.begin(), sortedLetters.end());
    for (const QChar &ch : sortedLetters)
        for (int i = 0; i < wheelCounts.value(ch); ++i)
            puzzle.letters.append(QString(ch));

    return puzzle;
}

// ─── Debug ────────────────────────────────────────────────────────────────────

void debugDumpPuzzle(const GeneratedPuzzle &puzzle, int attempts, qint64 elapsedMs)
{
    qDebug().noquote() << "================ generatePuzzle (crossword) ================";
    qDebug().noquote() << QString("  id=%1 grid=%2x%3 words=%4 score=%5 attempts=%6 elapsed=%7ms")
                              .arg(puzzle.puzzleId)
                              .arg(puzzle.rows)
                              .arg(puzzle.columns)
                              .arg(puzzle.words.size())
                              .arg(puzzle.score)
                              .arg(attempts)
                              .arg(elapsedMs);

    for (int r = 0; r < puzzle.rows; ++r)
    {
        QString line = QStringLiteral("    ");
        for (int c = 0; c < puzzle.columns; ++c)
        {
            const QChar ch = puzzle.grid[r][c];
            line += ch.isNull() ? QStringLiteral(" .") : QString(" %1").arg(ch);
        }
        qDebug().noquote() << line;
    }

    for (int i = 0; i < puzzle.words.size(); ++i)
    {
        const PlacedWord &w = puzzle.words[i];
        qDebug().noquote() << QString("  [#%1] id=%2 \"%3\" (%4) row=%5 col=%6 dir=%7")
                                  .arg(i)
                                  .arg(w.id)
                                  .arg(w.word)
                                  .arg(w.word.length())
                                  .arg(w.row)
                                  .arg(w.col)
                                  .arg(w.horizontal ? "H" : "V");
    }

    qDebug().noquote() << "============================================================";
}

} // namespace

// ─── PuzzleManager public API ─────────────────────────────────────────────────

PuzzleManager::PuzzleManager(QSharedPointer<StorageManager> storageManager)
    : m_storageManager(storageManager)
{
}

GeneratedPuzzle PuzzleManager::generatePuzzle(const QSet<int> &usedWordIds,
                                              const QSet<int> &unlockedImagesIds,
                                              LanguageLevel level, CharacterType characterType,
                                              int difficultyFactor)
{
    QElapsedTimer timer;
    timer.start();

    const DifficultyTargets dt = computeDifficulty(difficultyFactor);

    auto buildPool = [&](const QSet<int> &excludeIds)
    {
        QVector<WordEntry> pool =
            buildCandidatePool(*m_storageManager, level, excludeIds, dt.maxWordLength);

        if (pool.size() < dt.wordCount * 2)
        {
            QVector<WordEntry> recycled =
                buildCandidatePool(*m_storageManager, level, {}, dt.maxWordLength);
            if (recycled.size() > pool.size())
                pool = std::move(recycled);
        }

        if (pool.size() < dt.wordCount)
        {
            QVector<WordEntry> allLevels =
                buildCandidatePoolAllLevels(*m_storageManager, dt.maxWordLength);
            if (allLevels.size() > pool.size())
                pool = std::move(allLevels);
        }
        return pool;
    };

    QVector<WordEntry> pool = buildPool(usedWordIds);
    qInfo() << "[PuzzleManager] Candidate pool size:" << pool.size();

    auto sortLongestFirst = [](QVector<WordEntry> &v)
    {
        std::stable_sort(v.begin(), v.end(), [](const WordEntry &a, const WordEntry &b)
                         { return a.word.length() > b.word.length(); });
    };

    AttemptResult best;
    bool haveValidResult = false;
    int attemptsRun = 0;

    auto qualityKey = [&](const AttemptResult &r, int placed, int target)
    {
        return std::make_tuple(placed >= kMinValidWords ? 1 : 0, placed >= target ? 1 : 0,
                               r.totalCrossings, r.totalScore, placed);
    };

    auto runAttempts = [&](int target)
    {
        while (attemptsRun < kMaxAttempts && timer.elapsed() < kTimeBudgetMs)
        {
            ++attemptsRun;

            QVector<WordEntry> slice = pool;
            shuffleInPlace(slice);
            slice.resize(qMin(slice.size(), target * 3));
            sortLongestFirst(slice);

            AttemptResult attempt = tryGenerateOnce(slice, target, kGridRows, kGridColumns);
            const int placed = attempt.placed.size();

            if (attemptsRun == 1 ||
                qualityKey(attempt, placed, target) > qualityKey(best, best.placed.size(), target))
            {
                best = attempt;
                if (placed >= kMinValidWords)
                    haveValidResult = true;
            }

            const bool perfect = placed >= target && placed >= kMinValidWords &&
                                 attempt.totalCrossings >= placed - 1;
            if (perfect)
                break;
        }
    };

    for (int target = dt.wordCount; target >= kMinValidWords; --target)
    {
        runAttempts(target);
        if (haveValidResult)
            break;
    }

    if (!haveValidResult)
    {
        QVector<WordEntry> wider = buildPool({});
        if (wider.size() > pool.size())
        {
            qWarning() << "[PuzzleManager] Pool too small (" << pool.size()
                       << "), retrying with full dictionary (" << wider.size() << ")";
            pool = std::move(wider);
            attemptsRun = 0;
            for (int target = dt.wordCount; target >= kMinValidWords; --target)
            {
                runAttempts(target);
                if (haveValidResult)
                    break;
            }
        }
    }

    GeneratedPuzzle puzzle = finalizePuzzle(best, kGridRows, kGridColumns);

    if (puzzle.words.size() < kMinValidWords)
    {
        qWarning() << "[PuzzleManager] Failed to generate valid puzzle."
                   << "placed=" << puzzle.words.size() << "poolSize=" << pool.size()
                   << "attempts=" << attemptsRun << "difficulty=" << difficultyFactor;
        return GeneratedPuzzle{};
    }

    selectRewardImage(*m_storageManager, characterType, unlockedImagesIds, puzzle.imageId,
                      puzzle.imageSource);
    debugDumpPuzzle(puzzle, attemptsRun, timer.elapsed());
    return puzzle;
}
