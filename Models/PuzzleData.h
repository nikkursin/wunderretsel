#ifndef PUZZLEDATA_H
#define PUZZLEDATA_H

struct PuzzleCell {
    int index = 0;
    int row = 0;
    int column = 0;

    bool active = false;
    QChar letter;

    bool solved = false;
    QVector<int> wordIds;
};

struct PuzzleWord {
    int id = -1;
    QString word;

    int startRow = 0;
    int startColumn = 0;

    QString direction; // "horizontal" or "vertical"

    bool solved = false;
    QVector<int> cellIndexes;
};

struct GeneratedPuzzle {
    QString puzzleId;

    int imageId = -1;
    QString imageSource;

    int rows = 7;
    int columns = 7;

    QVector<PuzzleCell> cells;
    QVector<PuzzleWord> words;

    QStringList letters;
    QVector<int> solvedWordIds;
};

#endif // PUZZLEDATA_H
