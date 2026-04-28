#ifndef PUZZLEDATA_H
#define PUZZLEDATA_H

#include <QChar>
#include <QString>
#include <QStringList>
#include <QVector>

// Per-cell payload kept in `GeneratedPuzzle::cells` (a flat row-major array).
// This is the QML-friendly view: every cell of the grid is represented,
// `active=false` cells are rendered as gaps.
struct PuzzleCell {
    int index = 0;
    int row = 0;
    int column = 0;

    bool active = false;
    QChar letter;

    bool solved = false;

    // Indexes into GeneratedPuzzle::words for every word that crosses this
    // cell. With a real crossword a cell may belong to multiple words.
    QVector<int> wordIds;
};

// Authoritative record of a placed word. The crossword generator works
// against this struct exclusively; `PuzzleCell` is derived from it.
struct PlacedWord {
    int id = -1;            // dictionary id (WordEntry::id)
    QString word;           // canonical (uppercased) word

    int row = 0;            // start row
    int col = 0;            // start column
    bool horizontal = true; // true → grows along columns, false → along rows

    bool solved = false;
    QVector<int> cellIndexes;   // flat indexes into `cells`, in word order
};

struct GeneratedPuzzle {
    QString puzzleId;

    int imageId = -1;
    QString imageSource;

    int rows = 7;
    int columns = 7;

    // Authoritative 2D grid. `QChar()` (null char) means an empty cell.
    QVector<QVector<QChar>> grid;

    // Flat row-major view of the grid, ready for QML rendering.
    QVector<PuzzleCell> cells;

    // Words actually placed in the grid.
    QVector<PlacedWord> words;

    // Unique uppercase letters used in the puzzle (drives the letter wheel).
    QStringList letters;

    QVector<int> solvedWordIds;

    // Quality score of the chosen layout (sum of crossings * 10 minus
    // isolated-word penalties). Useful for telemetry / debug; not required
    // by the UI.
    int score = 0;
};

#endif // PUZZLEDATA_H
