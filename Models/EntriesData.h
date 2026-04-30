#ifndef ENTRIESDATA_H
#define ENTRIESDATA_H

#include "UserData.h"
#include <QString>

struct WordEntry
{
    int id = -1;
    QString word;
    LanguageLevel level;
};

struct ImageEntry
{
    int id = -1;
    QString source;
    CharacterType characterType;
};

#endif // ENTRIESDATA_H
