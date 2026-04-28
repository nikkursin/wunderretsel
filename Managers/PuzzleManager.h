#ifndef PUZZLEMANAGER_H
#define PUZZLEMANAGER_H

#include <QSet>
#include <QSharedPointer>
#include <QVector>

#include "Models/PuzzleData.h"
#include "Models/UserData.h"
#include "StorageManager.h"

// Generates crossword-style puzzles from the dictionary loaded by
// StorageManager. The algorithm is deterministic in shape (weighted
// full-grid scan with scoring) but seeded by QRandomGenerator so each
// run produces a different layout.
class PuzzleManager
{
public:
    explicit PuzzleManager(QSharedPointer<StorageManager> storageManager);

    // Build a fresh puzzle.
    //
    //   usedWordIds        – word ids the player has already encountered;
    //                        excluded from the candidate pool.
    //   unlockedImagesIds  – image ids already unlocked; excluded from the
    //                        reward pool.
    //   level              – player's current language level (A1..C2).
    //   characterType      – preferred reward image style.
    //   difficultyFactor   – how many puzzles the player has solved. Drives
    //                        word count and max word length.
    //
    // Returns a fully populated GeneratedPuzzle. If no valid layout is
    // found within the time budget the returned puzzle still contains as
    // many placed words as possible (never an exception, never a hang).
    GeneratedPuzzle generatePuzzle(const QSet<int>& usedWordIds,
                                   const QSet<int>& unlockedImagesIds,
                                   LanguageLevel level,
                                   CharacterType characterType,
                                   int difficultyFactor);

private:
    QSharedPointer<StorageManager> m_storageManager;
};

#endif // PUZZLEMANAGER_H
