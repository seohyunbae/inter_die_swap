#include <string>
#include <vector>

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "../GP/wlen.h"
#include "../GP/init_placement.h"
#include "legalizer.h"

#ifndef _LG_DB_
#define _LG_DB_

class Legalizer;

struct Cell
{
    std::string name;
    int width, height, weight;
    double x, y, optimalX, optimalY;

    Cell(std::string const &name, int const &width, int const &height, 
        double const &x, double const &y)
        : name(name), width(width), height(height), weight(width), 
        x(x), y(y), optimalX(x), optimalY(y) {}
};

struct Cluster
{
    double x, q;
    int width, weight;
    std::vector<Cell *> member;
    Cluster *predecessor;

    Cluster(double const &x, Cluster *predecessor, int const &weight, 
        double const &q, int const &width)
        : x(x), q(q), width(width), weight(weight), predecessor(predecessor) {}
};

struct SubRow
{
    int minX, maxX, freeWidth;
    Cluster *lastCluster;

    SubRow(int const &minX, int const&maxX)
        : minX(minX), maxX(maxX), freeWidth(maxX - minX), lastCluster(nullptr) {}
    inline void updateMinMax(int const &_minX, int const &_maxX)
    {
        this->minX = _minX;
        this->maxX = _maxX;
        this->freeWidth = _maxX - _minX;    
    }
};

struct Row
{
    int width, height, y;
    std::vector<SubRow *> subRows;

    Row(int const &width, int const &height, int const &y)
        : width(width), height(height), y(y) {}
};


struct TerminalGrid
{
    int numRows, numCols;
    int sizeX, sizeY;
    terminal_ptr ** grid;

    TerminalGrid(int const &_numRows, int const &_numCols, int const &_sizeX, int const &_sizeY) 
        : numRows(_numRows), numCols(_numCols), sizeX(_sizeX), sizeY(_sizeY) {}
    inline void generateTerminalGrid()
    {
        this->grid = (terminal_ptr **)malloc(sizeof(terminal_ptr*) *this->numRows);
        for (int i = 0; i < this->numRows; i++)
        {
            this->grid[i] = (terminal_ptr *)malloc(sizeof(terminal_ptr) *this->numCols);
            for (int j = 0; j < this->numCols; j++)
            {
                this->grid[i][j] = NULL;
            }
        }
    }
    inline void updateTerminalGrid(terminal_ptr terminal)
    {
        int rowIdx = 0, colIdx = 0;
        rowIdx = (int)((terminal->center.y - terminal->spacing) / this->sizeY);
        colIdx = (int)((terminal->center.x - terminal->spacing) / this->sizeX);

        this->grid[rowIdx][colIdx] = terminal;
    }
    inline void printGrid()
    {
        for (int i = 0; i < this->numRows; ++i)
        {
            for (int j = 0; j < this->numCols; ++j)
            {
                std::cout << this->grid[this->numRows - i - 1][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

struct LegalizerInput
{
    int maxDisplacement;
    std::vector<Cell *> cells;
    std::vector<Row *> rows;

    LegalizerInput() {}
    LegalizerInput(int const &maxDisplacement, std::vector<Cell *> const &cells, std::vector<Row *> const &rows)
        : maxDisplacement(maxDisplacement), cells(cells), rows(rows) {}
};

struct LegalizerDB
{
    std::vector<LegalizerInput *> die;
    LegalizerInput * terminal;
    std::unordered_map<std::string, Cell *> cell_hash;
    TerminalGrid * terminal_grid;

    // die size
    int width, height;
    int lowerLeftX, lowerLeftY;
    unsigned int upperRightX, upperRightY;

    void readDB(dataBase_ptr data);
    void writeDB(dataBase_ptr data);

    void readDie(die_ptr data, int dieNum);
    void readDieDB(dieDB_ptr data);
    void readInstanceDB(instanceDB_ptr data);
    void readTerminalDB(terminalDB_ptr data);
    void readGP(std::string const &filename);

    LegalizerDB() {}
    inline void updateDieSize(int const &_lowerLeftX, int const&_lowerLeftY, 
                            int const &_upperRightX, int const &_upperRightY)
    {
        this->lowerLeftX = _lowerLeftX;
        this->lowerLeftY = _lowerLeftY;
        this->upperRightX = _upperRightX;
        this->upperRightY = _upperRightY;
        this->width = _upperRightX - _lowerLeftX;
        this->height = _upperRightY - _lowerLeftY;
    }
};

// Leglization function
void legalize_local(LegalizerInput *input);
void legalize(dataBase_ptr data, std::string const &filename);

// Debug
void print_placement(LegalizerInput *input);
#endif