#ifndef _BIN_H
#define _BIN_H


#include "../DB/global.h"
#include "../DB/DataBase.h"


struct BIN{
    struct POS bin_coord;
    struct FPOS lowerLeft;
    struct FPOS upperRight;
    int numInst;
    double instArea;
    instance_ptr* inst_in_bin;
    int numTerm;
    double termArea;
    terminal_ptr* term_in_bin;
}; typedef struct BIN* bin_ptr;


struct BinDB{
    bin_ptr* binMat2d_top;
    bin_ptr* binMat2d_bot;
    bin_ptr* binMat2d_term;
    struct FSIZE binStep;
    struct SIZE numStep;
}; typedef struct BinDB* binDB_ptr;



struct POS getTermBinCoord(binDB_ptr binDB, terminal_ptr term);
struct POS getBinCoord(binDB_ptr binDB, instance_ptr inst);
binDB_ptr createBin(dataBase_ptr data, struct POS binSize);
void destroyBinDB(binDB_ptr rm_db);
void updateBinDB(dataBase_ptr data, binDB_ptr binDB);
double get_bin_ovfl(struct BIN bin, double target_den);
double get_global_ovfl(dataBase_ptr data, binDB_ptr binDB, double totalInstArea);


#endif