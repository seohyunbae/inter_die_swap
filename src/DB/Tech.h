#pragma once

#include "global.h"


struct Pin{
    char* pinName;
    double offsetX;
    double offsetY;

    struct Pin* next;
}; typedef struct Pin* pin_ptr;


struct CellSpec{
    char* masterCellName;
    char* techName;
    int numPins;
    int sizeX;
    int sizeY;

    pin_ptr pin_head;
    struct CellSpec* next;
}; typedef struct CellSpec* cellSpec_ptr;


struct MasterCell{
    char* masterCellName;
    cellSpec_ptr cellSpec_head;
    struct MasterCell* next;
}; typedef struct MasterCell* masterCell_ptr;


struct MasterCellHash{
    masterCell_ptr masterCell_head;
};  typedef struct MasterCellHash* masterCellHash_ptr;


struct MasterCellDB{
    int numTechs;
    int cur_bucket;
    int num_mastercell;
    masterCellHash_ptr hash_head;
}; typedef struct MasterCellDB* masterCellDB_ptr;


//List of functions
pin_ptr _create_pin(char* pinName, int offsetX, int offsetY);
void _destroy_pin(pin_ptr rm_pin);
cellSpec_ptr _create_cellSpec(char* masterCellName, char* techName, int numPins, int sizeX, int sizeY);
void _destroy_cellSpec(cellSpec_ptr rm_cellSpec);
masterCell_ptr _create_masterCell(char* masterCellName);
void _destroy_masterCell(masterCell_ptr rm_mc);
masterCellDB_ptr _create_masterCellDB(int numTechs);
void _destroy_masterCellDB(masterCellDB_ptr rm_db);

void _add_masterCell(masterCellDB_ptr target_db, masterCell_ptr target_masterCell);
masterCell_ptr _get_masterCell(masterCellDB_ptr target_db, char* masterCellName);
void _add_cellSpec(masterCellDB_ptr target_db, char* masterCellName, cellSpec_ptr target_cellSpec);
cellSpec_ptr _get_cellSpec(masterCellDB_ptr target_db, char* masterCellName, char* techName);
void _add_pin(masterCellDB_ptr target_db, char* masterCellName, char* techName, pin_ptr target_pin);

