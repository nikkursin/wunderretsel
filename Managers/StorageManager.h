#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>

#include "Models/EntriesData.h"
#include "Models/UserData.h"

class StorageManager
{
  public:
    StorageManager();

    UserData loadUserData();
    bool saveUser(const UserData &data);
    QVector<WordEntry> loadWordsByLevel(LanguageLevel level);
    QVector<ImageEntry> loadImagesByPreference(CharacterType characterType);

  private:
    QString m_userFilePath;
};

#endif // STORAGEMANAGER_H