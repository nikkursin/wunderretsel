#include "PuzzleManager.h"

#include <QChar>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <algorithm>

// =============================================================================
//  Tunables
// =============================================================================
namespace {

constexpr int kGridRows = 7;
constexpr int kGridColumns = 7;

// Gameplay requirement: never generate puzzles below 3 words.
constexpr int kMinWordCount = 3;
constexpr int kMaxWordCount = 6;

// Hard validity floor: reject anything below 3 placed words.
constexpr int kMinValidWordCount = 3;

// Word-length bounds. Spec mandates ≤ 8; we also need ≥ 2 so that a word
// can actually cross another.
constexpr int kMinWordLength = 2;
constexpr int kMaxWordLength = 8;

// Hard cap on the letter wheel. Each repeated letter in any single placed
// word needs its own wheel slot (otherwise the player can't trace e.g.
// "SEITE" – two E's), so we account for *multiplicity*, not unique letters.
constexpr int kMaxWheelLetters = 8;

// How many independent generation attempts we make and how long the whole
// generation is allowed to take (hard wall-clock cap, in milliseconds).
// Generous enough that one call almost always returns a valid layout
// without needing the caller to retry.
constexpr int kMaxAttempts = 200;
constexpr qint64 kTimeBudgetMs = 600;

// Scoring weights for placement decisions.
constexpr int kCrossingBonus = 10;   // per overlap with an existing letter
constexpr int kIsolationPenalty = -1;// when a word doesn't cross anything

// Direction vectors as required by the brief.
constexpr int dx[2] = {1, 0}; // horizontal step in column
constexpr int dy[2] = {0, 1}; // vertical step in row

} // namespace

// =============================================================================
//  Internal helpers
// =============================================================================
namespace {

// Per-attempt mutable state. Kept on the stack so we don't fight Qt's COW.
struct WorkingGrid {
    int rows = 0;
    int cols = 0;
    QVector<QVector<QChar>> cells; // [rows][cols], QChar() == empty
};

struct Placement {
    int row = 0;
    int col = 0;
    bool horizontal = true;
    int score = 0;
    int crossings = 0;
};

// Canonicalise a word for the grid. We uppercase per QString (so "ß" → "SS",
// matching how German players will read it) and reject anything outside
// the configured length window.
QString canonicalWord(const QString& raw)
{
    return raw.trimmed().toUpper();
}

// Reservoir-style Fisher-Yates shuffle on a QVector.
template <typename T>
void shuffleInPlace(QVector<T>& v)
{
    auto* rng = QRandomGenerator::global();
    for (int i = v.size() - 1; i > 0; --i) {
        const int j = rng->bounded(i + 1);
        if (j != i) std::swap(v[i], v[j]);
    }
}

// True iff (r, c) is inside the grid.
inline bool inBounds(const WorkingGrid& g, int r, int c)
{
    return r >= 0 && r < g.rows && c >= 0 && c < g.cols;
}

// Cell value or QChar() for cells outside the grid.
inline QChar cellAt(const WorkingGrid& g, int r, int c)
{
    return inBounds(g, r, c) ? g.cells[r][c] : QChar();
}

// Validate + score a single candidate placement.
//
// Returns std::nullopt if the placement is illegal. Otherwise returns the
// scored Placement. Validation rules:
//   1. Whole word must be inside bounds.
//   2. Every overlapped letter must match.
//   3. The cell immediately before the start and after the end of the word
//      (along the placement axis) must be empty — words can touch only by
//      crossing.
//   4. Empty cells of the new word must not be flanked, on the
//      perpendicular axis, by existing letters (no parallel-touching
//      pieces).
struct ScoreOpts {
    bool firstWord = false;     // first word: no crossings expected, no penalty
};

bool tryScorePlacement(const WorkingGrid& g,
                       const QString& word,
                       int row, int col, bool horizontal,
                       const ScoreOpts& opts,
                       Placement& out)
{
    const int len = word.length();
    if (len <= 0) return false;

    // Direction vectors (dx[0],dy[0]) horizontal vs (dx[1],dy[1]) vertical.
    const int dirIndex = horizontal ? 0 : 1;
    const int stepC = dx[dirIndex];
    const int stepR = dy[dirIndex];

    // Bounds.
    const int endR = row + stepR * (len - 1);
    const int endC = col + stepC * (len - 1);
    if (!inBounds(g, row, col) || !inBounds(g, endR, endC))
        return false;

    // Cell immediately before the word (no touching except by crossing).
    if (!cellAt(g, row - stepR, col - stepC).isNull()) return false;
    // Cell immediately after the word.
    if (!cellAt(g, endR + stepR, endC + stepC).isNull()) return false;

    int crossings = 0;
    for (int i = 0; i < len; ++i) {
        const int r = row + stepR * i;
        const int c = col + stepC * i;
        const QChar existing = g.cells[r][c];
        const QChar incoming = word.at(i);

        if (!existing.isNull()) {
            if (existing != incoming) return false;
            ++crossings;
            continue;
        }

        // Empty cell — make sure perpendicular neighbours are also empty,
        // otherwise the new word would be illegally adjacent to another.
        // Perpendicular axis is rotated 90°: swap step components.
        const int pStepR = stepC;
        const int pStepC = stepR;
        if (!cellAt(g, r + pStepR, c + pStepC).isNull()) return false;
        if (!cellAt(g, r - pStepR, c - pStepC).isNull()) return false;
    }

    int score;
    if (crossings > 0) {
        score = crossings * kCrossingBonus;
    } else if (opts.firstWord) {
        score = 0;                  // first word can't possibly cross anything
    } else {
        score = kIsolationPenalty;  // discouraged, but allowed as a fallback
    }

    out.row = row;
    out.col = col;
    out.horizontal = horizontal;
    out.score = score;
    out.crossings = crossings;
    return true;
}

// Mutate the grid by stamping a word into it.
void stampWord(WorkingGrid& g, const QString& word, const Placement& p)
{
    const int dirIndex = p.horizontal ? 0 : 1;
    const int stepC = dx[dirIndex];
    const int stepR = dy[dirIndex];
    for (int i = 0; i < word.length(); ++i) {
        g.cells[p.row + stepR * i][p.col + stepC * i] = word.at(i);
    }
}

// Find the highest-scoring placement for `word` on `grid`. Scans every
// (row, col, direction) — that's O(rows * cols * 2 * len) per word, which
// for our 7×7 board with ≤ 8-letter words is well under a millisecond.
//
// Returns false if no legal position was found.
bool findBestPlacement(const WorkingGrid& g,
                       const QString& word,
                       bool firstWord,
                       Placement& out)
{
    Placement best;
    bool haveBest = false;

    ScoreOpts opts;
    opts.firstWord = firstWord;

    // Full-grid scan over both directions. For the first word, score is
    // overridden with negative center-distance so central placements win.
    for (int r = 0; r < g.rows; ++r) {
        for (int c = 0; c < g.cols; ++c) {
            for (int d = 0; d < 2; ++d) {
                const bool horizontal = (d == 0);
                Placement cand;
                if (!tryScorePlacement(g, word, r, c, horizontal, opts, cand))
                    continue;

                if (firstWord) {
                    // Prefer placements that hug the grid centre.
                    const int len = word.length();
                    const int midR = horizontal ? r : r + len / 2;
                    const int midC = horizontal ? c + len / 2 : c;
                    const int dr = midR - g.rows / 2;
                    const int dc = midC - g.cols / 2;
                    cand.score = -(dr * dr + dc * dc); // closer == better
                }

                if (!haveBest || cand.score > best.score) {
                    best = cand;
                    haveBest = true;
                }
            }
        }
    }

    if (haveBest) out = best;
    return haveBest;
}

// One full generation attempt. Mutates `outPuzzle` and returns the total
// score (sum of placement scores). All-words-placed is required for the
// caller to consider this a "complete" attempt.
struct AttemptResult {
    QVector<PlacedWord> placed;
    int totalScore = 0;
    int totalCrossings = 0;
    int unplacedCount = 0;
};

AttemptResult tryGenerateOnce(const QVector<WordEntry>& shuffled,
                              int targetWordCount,
                              int rows, int cols)
{
    AttemptResult result;

    WorkingGrid g;
    g.rows = rows;
    g.cols = cols;
    g.cells.resize(rows);
    for (int r = 0; r < rows; ++r)
        g.cells[r].fill(QChar(), cols);

    // Wheel budget: the wheel must contain enough copies of each letter for
    // every placed word. Track the running max-per-letter count so we can
    // reject any candidate that would push the wheel above kMaxWheelLetters.
    QHash<QChar, int> wheelCounts;
    int wheelTotal = 0;

    auto countLetters = [](const QString& w) {
        QHash<QChar, int> h;
        for (const QChar& c : w) ++h[c];
        return h;
    };

    // Guard against the same canonical word appearing twice in a single
    // puzzle. The candidate pool is already deduped, but we defend in
    // depth here: dictionary curation drift, locale-sensitive uppercasing
    // (e.g. "ß" → "SS") or future pool sources could otherwise leak a
    // duplicate into the placed set, and even one collision is enough to
    // make the player see the same word twice.
    QSet<QString> placedCanonical;
    QSet<int> placedIds;

    int placedCount = 0;
    for (const WordEntry& entry : shuffled) {
        if (placedCount >= targetWordCount) break;

        const QString word = canonicalWord(entry.word);
        if (word.length() < kMinWordLength || word.length() > kMaxWordLength)
            continue;
        if (word.length() > qMin(rows, cols))
            continue;
        if (placedCanonical.contains(word))
            continue;
        if (entry.id >= 0 && placedIds.contains(entry.id))
            continue;

        // Pre-flight wheel-budget check: reject any word whose own letters
        // already exceed the wheel cap (e.g. an 8-letter word that would
        // need >8 distinct buttons because of repeats). Cheap; saves us
        // from running the placement scan for hopeless candidates.
        const QHash<QChar, int> wordCounts = countLetters(word);
        QHash<QChar, int> projectedWheel = wheelCounts;
        int projectedTotal = wheelTotal;
        for (auto it = wordCounts.constBegin(); it != wordCounts.constEnd(); ++it) {
            const int have = projectedWheel.value(it.key(), 0);
            if (it.value() > have) {
                projectedTotal += (it.value() - have);
                projectedWheel[it.key()] = it.value();
            }
        }
        if (projectedTotal > kMaxWheelLetters) {
            ++result.unplacedCount;
            continue;
        }

        Placement p;
        const bool firstWord = (placedCount == 0);
        if (!findBestPlacement(g, word, firstWord, p)) {
            ++result.unplacedCount;
            continue;
        }

        stampWord(g, word, p);

        PlacedWord pw;
        pw.id = entry.id;
        pw.word = word;
        pw.row = p.row;
        pw.col = p.col;
        pw.horizontal = p.horizontal;
        result.placed.append(pw);
        result.totalScore += p.score;
        result.totalCrossings += p.crossings;

        placedCanonical.insert(word);
        if (entry.id >= 0)
            placedIds.insert(entry.id);

        wheelCounts = projectedWheel;
        wheelTotal = projectedTotal;
        ++placedCount;
    }

    return result;
}

// Compute difficulty-scaled targets in one place.
struct DifficultyTargets {
    int wordCount;
    int maxWordLength;
};

DifficultyTargets computeDifficulty(int difficultyFactor)
{
    const int df = qMax(0, difficultyFactor);

    DifficultyTargets t;
    // Spec example: baseWordCount = 4; wordCount = base + difficulty.
    // We grow word count slowly and clamp to what the UI can render.
    t.wordCount = qBound(kMinWordCount,
                          kMinWordCount + df / 2,
                          kMaxWordCount);
    // maxLength = qMin(8, 4 + difficultyFactor)
    t.maxWordLength = qBound(kMinWordLength + 2,
                              4 + df,
                              kMaxWordLength);
    return t;
}

// Build the candidate pool: filter usedWords, length, dedupe by uppercased
// form (so "Reise" / "reise" don't both appear). Picks from the player's
// current level first, falls back to neighbouring levels.
QVector<WordEntry> buildCandidatePool(StorageManager& storage,
                                       LanguageLevel level,
                                       const QSet<int>& usedWordIds,
                                       int maxWordLength)
{
    auto loadFiltered = [&](LanguageLevel lvl) {
        QVector<WordEntry> v = storage.loadWordsByLevel(lvl);
        v.erase(std::remove_if(v.begin(), v.end(),
                               [&](const WordEntry& w) {
                                   const QString upper = canonicalWord(w.word);
                                   const int len = upper.length();
                                   return w.id < 0
                                          || usedWordIds.contains(w.id)
                                          || len < kMinWordLength
                                          || len > maxWordLength;
                               }),
                v.end());
        return v;
    };

    QVector<WordEntry> pool = loadFiltered(level);

    // Always sprinkle in adjacent levels (one step easier / harder) so
    // the pool stays large enough on small dictionaries.
    const int lvlInt = static_cast<int>(level);
    if (lvlInt > static_cast<int>(LanguageLevel::A1))
        pool += loadFiltered(static_cast<LanguageLevel>(lvlInt - 1));
    if (lvlInt < static_cast<int>(LanguageLevel::C2))
        pool += loadFiltered(static_cast<LanguageLevel>(lvlInt + 1));

    // Dedupe by uppercased form, preferring the first occurrence (current
    // level wins thanks to the order above).
    QSet<QString> seen;
    QVector<WordEntry> deduped;
    deduped.reserve(pool.size());
    for (const WordEntry& w : pool) {
        const QString upper = canonicalWord(w.word);
        if (seen.contains(upper)) continue;
        seen.insert(upper);
        deduped.append(w);
    }

    return deduped;
}

QVector<WordEntry> buildCandidatePoolAllLevels(StorageManager& storage,
                                               int maxWordLength)
{
    QVector<WordEntry> pool;
    for (int lvlInt = static_cast<int>(LanguageLevel::A1);
         lvlInt <= static_cast<int>(LanguageLevel::C2);
         ++lvlInt) {
        const auto lvl = static_cast<LanguageLevel>(lvlInt);
        QVector<WordEntry> v = storage.loadWordsByLevel(lvl);
        v.erase(std::remove_if(v.begin(), v.end(),
                               [&](const WordEntry& w) {
                                   const QString upper = canonicalWord(w.word);
                                   const int len = upper.length();
                                   return w.id < 0
                                          || len < kMinWordLength
                                          || len > maxWordLength;
                               }),
                v.end());
        pool += v;
    }

    QSet<QString> seen;
    QVector<WordEntry> deduped;
    deduped.reserve(pool.size());
    for (const WordEntry& w : pool) {
        const QString upper = canonicalWord(w.word);
        if (seen.contains(upper)) continue;
        seen.insert(upper);
        deduped.append(w);
    }
    return deduped;
}

// Select an unlocked image matching the player's preference. May return -1
// in `outId` when no eligible images exist; that leaves `imageSource`
// blank and the QML falls back to its sample image.
void selectRewardImage(StorageManager& storage,
                       CharacterType characterType,
                       const QSet<int>& unlockedImagesIds,
                       int& outId,
                       QString& outSource)
{
    outId = -1;
    outSource.clear();

    QVector<ImageEntry> images = storage.loadImagesByPreference(characterType);
    images.erase(std::remove_if(images.begin(), images.end(),
                                [&](const ImageEntry& e) {
                                    return unlockedImagesIds.contains(e.id);
                                }),
                 images.end());
    if (images.isEmpty()) return;

    const int idx = QRandomGenerator::global()->bounded(images.size());
    outId = images[idx].id;
    outSource = images[idx].source;
}

// Project the working result into the QML-friendly GeneratedPuzzle.
GeneratedPuzzle finalizePuzzle(const AttemptResult& attempt,
                               int rows, int cols)
{
    GeneratedPuzzle puzzle;
    puzzle.puzzleId = QString::number(QDateTime::currentMSecsSinceEpoch());
    puzzle.rows = rows;
    puzzle.columns = cols;
    puzzle.score = attempt.totalScore;

    // Authoritative 2D grid.
    puzzle.grid.resize(rows);
    for (int r = 0; r < rows; ++r) puzzle.grid[r].fill(QChar(), cols);

    // Stamp letters from placed words.
    for (const PlacedWord& pw : attempt.placed) {
        const int dirIndex = pw.horizontal ? 0 : 1;
        const int stepC = dx[dirIndex];
        const int stepR = dy[dirIndex];
        for (int i = 0; i < pw.word.length(); ++i) {
            puzzle.grid[pw.row + stepR * i][pw.col + stepC * i] = pw.word.at(i);
        }
    }

    // Flat cells view (row-major).
    puzzle.cells.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            PuzzleCell cell;
            cell.index = r * cols + c;
            cell.row = r;
            cell.column = c;
            const QChar ch = puzzle.grid[r][c];
            cell.active = !ch.isNull();
            cell.letter = ch;
            puzzle.cells.append(cell);
        }
    }

    // Copy placed words and back-fill cellIndexes / cell.wordIds.
    puzzle.words = attempt.placed;
    for (int wi = 0; wi < puzzle.words.size(); ++wi) {
        PlacedWord& pw = puzzle.words[wi];
        const int dirIndex = pw.horizontal ? 0 : 1;
        const int stepC = dx[dirIndex];
        const int stepR = dy[dirIndex];
        pw.cellIndexes.reserve(pw.word.length());
        for (int i = 0; i < pw.word.length(); ++i) {
            const int r = pw.row + stepR * i;
            const int c = pw.col + stepC * i;
            const int flatIndex = r * cols + c;
            pw.cellIndexes.append(flatIndex);

            PuzzleCell& cell = puzzle.cells[flatIndex];
            if (!cell.wordIds.contains(wi))
                cell.wordIds.append(wi);
        }
    }

    // Wheel letters with multiplicity: for every letter we keep the maximum
    // number of times it appears in any single placed word. That guarantees
    // the player can trace each word (e.g. "SEITE" needs two E buttons) and
    // matches exactly what tryGenerateOnce budgeted against kMaxWheelLetters.
    QHash<QChar, int> wheelCounts;
    for (const PlacedWord& pw : puzzle.words) {
        QHash<QChar, int> per;
        for (const QChar& ch : pw.word) ++per[ch];
        for (auto it = per.constBegin(); it != per.constEnd(); ++it) {
            wheelCounts[it.key()] = qMax(wheelCounts.value(it.key(), 0),
                                          it.value());
        }
    }

    // Stable, sorted order so the wheel layout doesn't shuffle between
    // re-renders of the same puzzle. (Players still get a Shuffle button.)
    QList<QChar> orderedLetters = wheelCounts.keys();
    std::sort(orderedLetters.begin(), orderedLetters.end());
    for (const QChar& ch : orderedLetters) {
        const int count = wheelCounts.value(ch);
        for (int i = 0; i < count; ++i)
            puzzle.letters.append(QString(ch));
    }

    return puzzle;
}

void debugDumpPuzzle(const GeneratedPuzzle& puzzle, int attemptsRun, qint64 elapsedMs)
{
    qDebug().noquote() << "================ generatePuzzle (crossword) ================";
    qDebug().noquote() << QString("  id=%1 grid=%2x%3 words=%4 score=%5 attempts=%6 elapsed=%7ms")
                              .arg(puzzle.puzzleId)
                              .arg(puzzle.rows)
                              .arg(puzzle.columns)
                              .arg(puzzle.words.size())
                              .arg(puzzle.score)
                              .arg(attemptsRun)
                              .arg(elapsedMs);
    for (int r = 0; r < puzzle.rows; ++r) {
        QString line = QStringLiteral("    ");
        for (int c = 0; c < puzzle.columns; ++c) {
            const QChar ch = puzzle.grid[r][c];
            line += ch.isNull() ? QStringLiteral(" .") : QString(" %1").arg(ch);
        }
        qDebug().noquote() << line;
    }
    for (int wi = 0; wi < puzzle.words.size(); ++wi) {
        const PlacedWord& w = puzzle.words[wi];
        qDebug().noquote() << QString("  [#%1] id=%2 \"%3\" (%4) row=%5 col=%6 dir=%7")
                                  .arg(wi)
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

// =============================================================================
//  Public API
// =============================================================================
PuzzleManager::PuzzleManager(QSharedPointer<StorageManager> storageManager)
    : m_storageManager(storageManager) {}

GeneratedPuzzle PuzzleManager::generatePuzzle(const QSet<int>& usedWordIds,
                                               const QSet<int>& unlockedImagesIds,
                                               LanguageLevel level,
                                               CharacterType characterType,
                                               int difficultyFactor)
{
    QElapsedTimer timer;
    timer.start();

    const DifficultyTargets dt = computeDifficulty(difficultyFactor);

    auto buildPool = [&](const QSet<int>& excludeIds) {
        QVector<WordEntry> pool = buildCandidatePool(*m_storageManager,
                                                      level,
                                                      excludeIds,
                                                      dt.maxWordLength);

        // If the player has used so many words that the pool can no
        // longer support a full puzzle, recycle the dictionary. We
        // never want to ship an empty puzzle to the UI.
        if (pool.size() < dt.wordCount * 2) {
            QVector<WordEntry> recycled = buildCandidatePool(*m_storageManager,
                                                              level,
                                                              /*usedWordIds=*/{},
                                                              dt.maxWordLength);
            if (recycled.size() > pool.size())
                pool = std::move(recycled);
        }

        // If level-specific resource loading failed or produced too few
        // entries, widen the search across all levels so gameplay never
        // stalls.
        if (pool.size() < dt.wordCount) {
            QVector<WordEntry> allLevels = buildCandidatePoolAllLevels(
                *m_storageManager, dt.maxWordLength);
            if (allLevels.size() > pool.size())
                pool = std::move(allLevels);
        }
        return pool;
    };

    QVector<WordEntry> pool = buildPool(usedWordIds);
    qInfo() << "[PuzzleManager] Candidate pool size (preferred levels):" << pool.size();

    // Always sort longer-first inside an attempt: long words anchor better
    // crossings and fail fast when they don't fit, which speeds up retries.
    auto sortLongFirst = [](QVector<WordEntry>& v) {
        std::stable_sort(v.begin(), v.end(),
                         [](const WordEntry& a, const WordEntry& b) {
                             return a.word.length() > b.word.length();
                         });
    };

    AttemptResult bestAttempt;
    bool haveAttempt = false;
    bool haveValidAttempt = false; // at least one attempt with ≥ kMinValidWordCount words
    int attemptsRun = 0;

    auto qualityKey = [&](const AttemptResult& r, int p, int targetWordCount) {
        // Tuple ordered for std::tuple comparison (higher is better):
        //   1. valid?      (>= kMinValidWordCount placed)   – non-negotiable
        //   2. complete?   (>= targetWordCount placed)      – preferred
        //   3. crossings   – more interconnected layouts win
        //   4. raw score   – tie-breaker
        //   5. placed      – more words is better
        const bool valid = (p >= kMinValidWordCount);
        const bool complete = (p >= targetWordCount);
        return std::make_tuple(valid ? 1 : 0,
                               complete ? 1 : 0,
                               r.totalCrossings,
                               r.totalScore,
                               p);
    };

    // Multiple attempts; keep the highest-scoring VALID one. If no valid
    // attempt exists we'll widen the pool below and retry. We bail out
    // once we've found a "perfect" layout (all words placed, every word
    // crosses at least one other) because more attempts can't beat that.
    auto runAttempts = [&](int targetWordCount) {
        while (attemptsRun < kMaxAttempts && timer.elapsed() < kTimeBudgetMs) {
            ++attemptsRun;

            QVector<WordEntry> attemptPool = pool;
            shuffleInPlace(attemptPool);

            // Take a generous slice (up to 3× target) so we have room to
            // reject words that don't fit; sort longest-first inside that
            // slice for good anchoring.
            const int sliceSize = qMin(attemptPool.size(), targetWordCount * 3);
            attemptPool.resize(sliceSize);
            sortLongFirst(attemptPool);

            AttemptResult res = tryGenerateOnce(attemptPool,
                                                 targetWordCount,
                                                 kGridRows, kGridColumns);

            const int placed = res.placed.size();
            const bool isValid = (placed >= kMinValidWordCount);

            if (!haveAttempt
                || qualityKey(res, placed, targetWordCount)
                       > qualityKey(bestAttempt, bestAttempt.placed.size(), targetWordCount)) {
                bestAttempt = res;
                haveAttempt = true;
                if (isValid) haveValidAttempt = true;
            }

            // Perfect: every word placed, and every word past the first
            // crosses at least one other (totalCrossings >= placed - 1).
            if (isValid
                && placed >= targetWordCount
                && res.totalCrossings >= placed - 1) {
                break;
            }
        }
    };

    // If a high difficulty target is too tight for the current pool/grid,
    // step the target down until we still satisfy the gameplay floor
    // (>= kMinValidWordCount words). This prevents "can't start game"
    // dead-ends at higher solvedPuzzleCount values.
    for (int target = dt.wordCount; target >= kMinValidWordCount; --target) {
        runAttempts(target);
        if (haveValidAttempt)
            break;
    }

    // Validity safety net: if every attempt produced < kMinValidWordCount
    // words, the pool is probably too narrow for the current grid /
    // difficulty. Recycle the dictionary (drop the usedWordIds filter)
    // and run another batch of attempts. This is the same recovery path
    // AppStateManager would otherwise have to take after the fact, kept
    // inside PuzzleManager so the public contract is ">= 3 words or a
    // logged best-effort fallback".
    if (!haveValidAttempt) {
        QVector<WordEntry> wider = buildPool(/*excludeIds=*/{});
        if (wider.size() > pool.size()) {
            qWarning() << "[PuzzleManager] No valid attempt with current pool ("
                       << pool.size() << "). Retrying with recycled dictionary ("
                       << wider.size() << ").";
            pool = std::move(wider);
            attemptsRun = 0;
            for (int target = dt.wordCount; target >= kMinValidWordCount; --target) {
                runAttempts(target);
                if (haveValidAttempt)
                    break;
            }
        }
    }

    // Hand-off to GeneratedPuzzle.
    GeneratedPuzzle puzzle = finalizePuzzle(bestAttempt, kGridRows, kGridColumns);

    if (puzzle.words.size() < kMinValidWordCount) {
        qWarning() << "[PuzzleManager] Generated invalid puzzle (<"
                   << kMinValidWordCount << " words). Returning empty puzzle."
                   << "placed=" << puzzle.words.size()
                   << "poolSize=" << pool.size()
                   << "attemptsRun=" << attemptsRun
                   << "difficultyFactor=" << difficultyFactor;
        return GeneratedPuzzle{};
    }

    selectRewardImage(*m_storageManager,
                      characterType,
                      unlockedImagesIds,
                      puzzle.imageId,
                      puzzle.imageSource);

    debugDumpPuzzle(puzzle, attemptsRun, timer.elapsed());
    return puzzle;
}
