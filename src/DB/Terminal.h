#pragma once

#include "global.h"
#include "Net.h"


struct Terminal{
    int sizeX;
    int sizeY;
    int spacing;

    int termIndex;
    FCOORD center;
    
    char* netName;
    net_ptr term_net;
    struct Terminal* sweep_next;
    struct Terminal* next;

    // Global placement
    // For charge grad calc
    struct POS binCoord;
    struct POS fft_coord_min, fft_coord_max;
    struct FPOS charge_grad;
    // For wlen grad calc
    struct FPOS e1, e2;
    struct POS flg1, flg2;
    struct FPOS wlen_grad;
    struct FPOS grad;
}; typedef struct Terminal* terminal_ptr;

struct terminalGrid
{
    int numRows, numCols;
    int sizeX, sizeY;
    terminal_ptr ** grid;

    terminalGrid(int const &_numRows, int const &_numCols, int const &_sizeX, int const &_sizeY) 
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

struct TerminalHash{
    terminal_ptr terminal_head;
}; typedef struct TerminalHash* terminalHash_ptr;


struct TerminalDB{
    int numTerminals;
    int cur_bucket;

    int sizeX;
    int sizeY;
    int spacing;
    int area;

    int sizeX_w_spacing;
    int sizeY_w_spacing;
    int area_w_spacing;

    terminal_ptr* term_array;
    terminalHash_ptr hash_head;
    terminalGrid * terminal_grid;
}; typedef struct TerminalDB* terminalDB_ptr;


//List of functions
terminal_ptr _create_terminal(int sizeX, int sizeY, int spacing, net_ptr target_net);
void _destroy_terminal(terminal_ptr rm_term);
terminalDB_ptr _create_terminalDB(int sizeX, int sizeY, int spacing);
void _destroy_terminalDB(terminalDB_ptr rm_db);

void _add_terminal(terminalDB_ptr target_db, terminal_ptr target_term);
void _remove_terminal(terminalDB_ptr target_db, terminal_ptr target_term);
terminal_ptr _get_terminal(terminalDB_ptr target_db, char* netName);
void _update_terminal(terminalDB_ptr target_db, char* netName, FCOORD coord);
