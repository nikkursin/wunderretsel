#ifndef PUZZLEDATA_H
#define PUZZLEDATA_H

#include <QChar>
#include <QString>
#include <QStringList>
#include <QVector>

struct PuzzleCell
{
    int index = 0;
    int row = 0;
    int column = 0;

    bool active = false;
    QChar letter;

    bool solved = false;

    QVector<int> wordIds;
};

struct PlacedWord
{
    int id = -1;
    QString word;

    int row = 0;
    int col = 0;
    bool horizontal = true;

    bool solved = false;
    QVector<int> cellIndexes;
};

struct GeneratedPuzzle
{
    QString puzzleId;

    int imageId = -1;
    QString imageSource;

    int rows = 7;
    int columns = 7;

    QVector<QVector<QChar>> grid;

    QVector<PuzzleCell> cells;

    QVector<PlacedWord> words;

    QStringList letters;

    QVector<int> solvedWordIds;

    int score = 0;
};

#endif // PUZZLEDATA_H
