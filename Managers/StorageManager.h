#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>

#include "Models/UserData.h"


class StorageManager
{
  public:
    StorageManager();

    UserData loadUserData();

    bool saveUser(const UserData& data);

  private:
    QString m_userFilePath;

    LanguageLevel languageLevelFromString(const QString& value);

    CharacterType characterTypeFromString(const QString& value);
};

#endif // STORAGEMANAGER_H
