#include "bin.h"


struct POS getBinCoord(binDB_ptr binDB, instance_ptr inst)
{
    struct FCOORD center = inst->center;
    struct FSIZE binStep = binDB->binStep;
    struct POS binCoord;
    binCoord.x = (int)floor(center.x / binStep.x);
    binCoord.y = (int)floor(center.y / binStep.y);
    return binCoord;
}


struct POS getTermBinCoord(binDB_ptr binDB, terminal_ptr term)
{
    struct FCOORD center = term->center;
    struct FSIZE binStep = binDB->binStep;
    struct POS binCoord;
    binCoord.x = (int)floor(center.x / binStep.x);
    binCoord.y = (int)floor(center.y / binStep.y);
    return binCoord;
}


binDB_ptr createBin(dataBase_ptr data, struct POS binSize)
{
    // struct SIZE binSize;
    binDB_ptr new_db = (binDB_ptr)malloc(sizeof(struct BinDB));
    // binSize.x = binSizeX;
    // binSize.y = binSizeY;
    new_db->numStep.x = (int)floor((data->dieDB->upperRightX - data->dieDB->lowerLeftX) / (double)binSize.x);
    new_db->numStep.y = (int)floor((data->dieDB->upperRightY - data->dieDB->lowerLeftY) / (double)binSize.y);
    if (new_db->numStep.x == 0) new_db->numStep.x = 4;
    else new_db->numStep.x = std::min(minBinNumStepX, new_db->numStep.x);
    if (new_db->numStep.y == 0) new_db->numStep.y = 4;
    else new_db->numStep.y = std::min(minBinNumStepY, new_db->numStep.y);
    new_db->binStep.x = (data->dieDB->upperRightX - data->dieDB->lowerLeftX) / (double)(new_db->numStep.x);
    new_db->binStep.y = (data->dieDB->upperRightY - data->dieDB->lowerLeftY) / (double)(new_db->numStep.y);
    new_db->binMat2d_top = (bin_ptr*)malloc(sizeof(bin_ptr) * (new_db->numStep.x));
    new_db->binMat2d_bot = (bin_ptr*)malloc(sizeof(bin_ptr) * (new_db->numStep.x));
    new_db->binMat2d_term = (bin_ptr*)malloc(sizeof(bin_ptr) * (new_db->numStep.x));
    for (int x = 0; x < new_db->numStep.x; x++)
    {
        double lowerLeftX = data->dieDB->lowerLeftX + x * new_db->binStep.x;
        double upperRightX = lowerLeftX + new_db->binStep.x;
        new_db->binMat2d_top[x] = (bin_ptr)malloc(sizeof(struct BIN) * (new_db->numStep.y));
        new_db->binMat2d_bot[x] = (bin_ptr)malloc(sizeof(struct BIN) * (new_db->numStep.y));
        new_db->binMat2d_term[x] = (bin_ptr)malloc(sizeof(struct BIN) * (new_db->numStep.y));
        for (int y = 0; y < new_db->numStep.y; y++)
        {
            // Top die
            new_db->binMat2d_top[x][y].bin_coord.x = x;
            new_db->binMat2d_top[x][y].bin_coord.y = y;
            // Bin boundary setting
            new_db->binMat2d_top[x][y].lowerLeft.x = lowerLeftX;
            new_db->binMat2d_top[x][y].lowerLeft.y = data->dieDB->lowerLeftY + y * new_db->binStep.y;
            new_db->binMat2d_top[x][y].upperRight.x = upperRightX;
            new_db->binMat2d_top[x][y].upperRight.y = new_db->binMat2d_top[x][y].lowerLeft.y + new_db->binStep.y;
            // Instance array initially null. 
            new_db->binMat2d_top[x][y].numInst = 0;
            new_db->binMat2d_top[x][y].instArea = 0;
            new_db->binMat2d_top[x][y].inst_in_bin = NULL;

            // Bot die
            new_db->binMat2d_bot[x][y].bin_coord.x = x;
            new_db->binMat2d_bot[x][y].bin_coord.y = y;
            // Bin boundary setting
            new_db->binMat2d_bot[x][y].lowerLeft.x = lowerLeftX;
            new_db->binMat2d_bot[x][y].lowerLeft.y = data->dieDB->lowerLeftY + y * new_db->binStep.y;
            new_db->binMat2d_bot[x][y].upperRight.x = upperRightX;
            new_db->binMat2d_bot[x][y].upperRight.y = new_db->binMat2d_bot[x][y].lowerLeft.y + new_db->binStep.y;
            // Instance array initially null. 
            new_db->binMat2d_bot[x][y].numInst = 0;
            new_db->binMat2d_bot[x][y].instArea = 0;
            new_db->binMat2d_bot[x][y].inst_in_bin = NULL;

            // Terminal
            new_db->binMat2d_term[x][y].bin_coord.x = x;
            new_db->binMat2d_term[x][y].bin_coord.y = y;
            // Bin boundary setting
            new_db->binMat2d_term[x][y].lowerLeft.x = lowerLeftX;
            new_db->binMat2d_term[x][y].lowerLeft.y = data->dieDB->lowerLeftY + y * new_db->binStep.y;
            new_db->binMat2d_term[x][y].upperRight.x = upperRightX;
            new_db->binMat2d_term[x][y].upperRight.y = new_db->binMat2d_term[x][y].lowerLeft.y + new_db->binStep.y;
            // Instance array initially null. 
            new_db->binMat2d_term[x][y].numTerm = 0;
            new_db->binMat2d_term[x][y].termArea = 0;
            new_db->binMat2d_term[x][y].term_in_bin = NULL;
        }
    }
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = data->instanceDB->inst_array[i];
        struct POS curPos = getBinCoord(new_db, curInst);
        if (curInst->dieNum == 0)
        {
            new_db->binMat2d_top[curPos.x][curPos.y].numInst++;
            new_db->binMat2d_top[curPos.x][curPos.y].instArea += (double)curInst->area;
            instance_ptr* inst_in_bin = new_db->binMat2d_top[curPos.x][curPos.y].inst_in_bin;
            unsigned int mem = new_db->binMat2d_top[curPos.x][curPos.y].numInst * sizeof(instance_ptr);
            new_db->binMat2d_top[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)realloc(inst_in_bin, mem);
            curInst->binCoord.x = curPos.x;
            curInst->binCoord.y = curPos.y;
        }
        else
        {
            new_db->binMat2d_bot[curPos.x][curPos.y].numInst++;
            new_db->binMat2d_bot[curPos.x][curPos.y].instArea += (double)curInst->area;
            instance_ptr* inst_in_bin = new_db->binMat2d_bot[curPos.x][curPos.y].inst_in_bin;
            unsigned int mem = new_db->binMat2d_bot[curPos.x][curPos.y].numInst * sizeof(instance_ptr);
            new_db->binMat2d_bot[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)realloc(inst_in_bin, mem);
            curInst->binCoord.x = curPos.x;
            curInst->binCoord.y = curPos.y;
        }
    }
    int expanded_size = data->terminalDB->sizeX_w_spacing * data->terminalDB->sizeY_w_spacing;
    for (int i = 0; i < data->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = data->terminalDB->term_array[i];
        struct POS curPos = getTermBinCoord(new_db, curTerm);
        new_db->binMat2d_term[curPos.x][curPos.y].numTerm++;
        new_db->binMat2d_term[curPos.x][curPos.y].termArea += (double)expanded_size;
        terminal_ptr* term_in_bin = new_db->binMat2d_term[curPos.x][curPos.y].term_in_bin;
        unsigned int mem = new_db->binMat2d_term[curPos.x][curPos.y].numTerm * sizeof(terminal_ptr);
        new_db->binMat2d_term[curPos.x][curPos.y].term_in_bin = (terminal_ptr*)realloc(term_in_bin, mem);
        curTerm->binCoord.x = curPos.x;
        curTerm->binCoord.y = curPos.y;
    }
    return new_db;
}


void destroyBinDB(binDB_ptr rm_db)
{
    int X = rm_db->numStep.x;
    int Y = rm_db->numStep.y;
    for (int x = 0; x < X; x++)
    {
        for (int y = 0; y < Y; y++)
        {
            if (rm_db->binMat2d_top[x][y].inst_in_bin) free(rm_db->binMat2d_top[x][y].inst_in_bin);
            if (rm_db->binMat2d_bot[x][y].inst_in_bin) free(rm_db->binMat2d_bot[x][y].inst_in_bin);
            if (rm_db->binMat2d_term[x][y].term_in_bin) free(rm_db->binMat2d_term[x][y].term_in_bin);
        }
        free(rm_db->binMat2d_top[x]);
        free(rm_db->binMat2d_bot[x]);
        free(rm_db->binMat2d_term[x]);
    }
    free(rm_db->binMat2d_top);
    free(rm_db->binMat2d_bot);
    free(rm_db->binMat2d_term);
    free(rm_db);
}


void updateBinDB(dataBase_ptr data, binDB_ptr binDB)
{
    // After each iteration, instance can be placed in different bin. 
    // Since it takes time to pop instance array from each bin, reconstruction of instance array is more efficient way. 
    instance_ptr curInst;
    for (int x = 0; x < binDB->numStep.x; x++)
    {
        for (int y = 0; y < binDB->numStep.y; y++)
        {
            if (binDB->binMat2d_top[x][y].inst_in_bin) {
                free(binDB->binMat2d_top[x][y].inst_in_bin);
                binDB->binMat2d_top[x][y].inst_in_bin = NULL;
                binDB->binMat2d_top[x][y].numInst = 0;
                binDB->binMat2d_top[x][y].instArea = 0;
            }
            if (binDB->binMat2d_bot[x][y].inst_in_bin) {
                free(binDB->binMat2d_bot[x][y].inst_in_bin);
                binDB->binMat2d_bot[x][y].inst_in_bin = NULL;
                binDB->binMat2d_bot[x][y].numInst = 0;
                binDB->binMat2d_bot[x][y].instArea = 0;
            }
            if (binDB->binMat2d_term[x][y].term_in_bin){
                free(binDB->binMat2d_term[x][y].term_in_bin);
                binDB->binMat2d_term[x][y].term_in_bin = NULL;
                binDB->binMat2d_term[x][y].numTerm = 0;
                binDB->binMat2d_term[x][y].termArea = 0;
            }
        }
    }
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];
        struct POS curPos = getBinCoord(binDB, curInst);
        if (curInst->dieNum == 0)
        {
            binDB->binMat2d_top[curPos.x][curPos.y].numInst++;
            binDB->binMat2d_top[curPos.x][curPos.y].instArea += (double)curInst->area;

            instance_ptr* inst_in_bin = binDB->binMat2d_top[curPos.x][curPos.y].inst_in_bin;
            unsigned int mem = binDB->binMat2d_top[curPos.x][curPos.y].numInst * sizeof(instance_ptr);
            if (inst_in_bin == NULL) 
            {
                binDB->binMat2d_top[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)malloc(sizeof(instance_ptr));
                binDB->binMat2d_top[curPos.x][curPos.y].inst_in_bin[0] = curInst;
            }
            else 
            {
                int index = binDB->binMat2d_top[curPos.x][curPos.y].numInst - 1;
                binDB->binMat2d_top[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)realloc(inst_in_bin, mem);
                binDB->binMat2d_top[curPos.x][curPos.y].inst_in_bin[index] = curInst;
            }
        }
        else
        {
            binDB->binMat2d_bot[curPos.x][curPos.y].numInst++;
            binDB->binMat2d_bot[curPos.x][curPos.y].instArea += (double)curInst->area;

            instance_ptr* inst_in_bin = binDB->binMat2d_bot[curPos.x][curPos.y].inst_in_bin;
            unsigned int mem = binDB->binMat2d_bot[curPos.x][curPos.y].numInst * sizeof(instance_ptr);
            if (inst_in_bin == NULL) 
            {
                binDB->binMat2d_bot[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)malloc(sizeof(instance_ptr));
                binDB->binMat2d_bot[curPos.x][curPos.y].inst_in_bin[0] = curInst;
            }
            else 
            {
                int index = binDB->binMat2d_bot[curPos.x][curPos.y].numInst - 1;
                binDB->binMat2d_bot[curPos.x][curPos.y].inst_in_bin = (instance_ptr*)realloc(inst_in_bin, mem);
                binDB->binMat2d_bot[curPos.x][curPos.y].inst_in_bin[index] = curInst;
            }
        }
        curInst->binCoord.x = curPos.x;
        curInst->binCoord.y = curPos.y;
    }
    int expanded_size = data->terminalDB->sizeX_w_spacing * data->terminalDB->sizeY_w_spacing;
    for (int i = 0; i < data->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = data->terminalDB->term_array[i];
        struct POS curPos = getTermBinCoord(binDB, curTerm);
        binDB->binMat2d_term[curPos.x][curPos.y].numTerm++;
        binDB->binMat2d_term[curPos.x][curPos.y].termArea += expanded_size;
        terminal_ptr* term_in_bin = binDB->binMat2d_term[curPos.x][curPos.y].term_in_bin;
        unsigned int mem = binDB->binMat2d_term[curPos.x][curPos.y].numTerm * sizeof(terminal_ptr);
        if (term_in_bin == NULL)
        {
            binDB->binMat2d_term[curPos.x][curPos.y].term_in_bin = (terminal_ptr*)malloc(sizeof(terminal_ptr));
            binDB->binMat2d_term[curPos.x][curPos.y].term_in_bin[0] = curTerm;
        }
        else
        {
            int index = binDB->binMat2d_term[curPos.x][curPos.y].numTerm - 1;
            binDB->binMat2d_term[curPos.x][curPos.y].term_in_bin = (terminal_ptr*)realloc(term_in_bin, mem);
            binDB->binMat2d_term[curPos.x][curPos.y].term_in_bin[index] = curTerm;
        }
        curTerm->binCoord.x = curPos.x;
        curTerm->binCoord.y = curPos.y;
    }
}


double get_bin_ovfl(struct BIN bin, double target_den)
{
    using namespace std;
    double binArea = (bin.upperRight.x - bin.lowerLeft.x) * (bin.upperRight.y - bin.lowerLeft.y);
    double density = bin.instArea / binArea;
    double ovfl = max(0.0, density - target_den);
    // if (ovfl > 1)printf("ovfl = %3lf\n", ovfl);
    return ovfl * binArea;
}


double get_global_ovfl(dataBase_ptr data, binDB_ptr binDB, double totalInstArea)
{
    double Govfl = 0.0;
    double top_target = data->dieDB->top_die->targetUtil;
    double bot_target = data->dieDB->bot_die->targetUtil;
    for (int x = 0; x < binDB->numStep.x; x++)
    {
        for (int y = 0; y < binDB->numStep.y; y++)
        {
            Govfl += get_bin_ovfl(binDB->binMat2d_top[x][y], top_target);
            Govfl += get_bin_ovfl(binDB->binMat2d_bot[x][y], bot_target);
        }
    }
    return Govfl / totalInstArea;
}