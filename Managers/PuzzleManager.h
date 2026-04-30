#ifndef PUZZLEMANAGER_H
#define PUZZLEMANAGER_H

#include <QSet>
#include <QSharedPointer>
#include <QVector>

#include "Models/PuzzleData.h"
#include "Models/UserData.h"
#include "StorageManager.h"

class PuzzleManager
{
  public:
    explicit PuzzleManager(QSharedPointer<StorageManager> storageManager);

    GeneratedPuzzle generatePuzzle(const QSet<int> &usedWordIds, const QSet<int> &unlockedImagesIds,
                                   LanguageLevel level, CharacterType characterType,
                                   int difficultyFactor);

  private:
    QSharedPointer<StorageManager> m_storageManager;
};

#endif // PUZZLEMANAGER_H