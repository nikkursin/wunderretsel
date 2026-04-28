#ifndef PUZZLEGENERATOR_H
#define PUZZLEGENERATOR_H

#include <QSharedPointer>
#include <QVector>

#include "StorageManager.h"
#include "Models/PuzzleData.h"
#include "Models/UserData.h"

class PuzzleManager
{
  public:
    PuzzleManager(QSharedPointer<StorageManager> storageManager);

    GeneratedPuzzle generatePuzzle(QVector<int> usedWordsIndexes, QVector<int> unlockedImages, LanguageLevel level, CharacterType characterType);

  private:

    QSharedPointer<StorageManager> m_storageManager;
};

#endif // PUZZLEGENERATOR_H
