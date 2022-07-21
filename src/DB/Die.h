#pragma once

#include "Instance.h"
#include "Tech.h"
#include "global.h"


struct DieInstanceHash{
    instance_ptr dieInstance_head;
}; typedef struct DieInstanceHash* dieInstanceHash_ptr;


struct Die{
    char* dieName;
    char* techName;

    // die size
    int lowerLeftX;
    int lowerLeftY;
    unsigned int upperRightX;
    unsigned int upperRightY;
    // target utilization
    int targetUtil;
    // placement row
    int startX;
    int startY;
    int rowLength;
    int rowHeight;
    int repeatCount;

    unsigned int dieArea;
    
    int cur_bucket;
    int numInsts;
    unsigned int curArea;
    double curUtil;

    //for die partition
    unsigned int targetArea;
    dieInstanceHash_ptr hash_head;
}; typedef struct Die* die_ptr;


struct DieDB{
    int lowerLeftX;
    int lowerLeftY;
    int upperRightX;
    int upperRightY;
    double centerX;
    double centerY;
   unsigned int dieArea;

    die_ptr top_die;
    die_ptr bot_die;
}; typedef struct DieDB* dieDB_ptr;


die_ptr _create_die(char* dieName, char* techName,
                    int lowerLeftX, int lowerLeftY,
                    int upperRightX, int upperRightY,
                    int targetUtil,
                    int startX, int startY,
                    int rowLength, int rowHeight,
                    int repeatCount);
void _destroy_die(die_ptr rm_die);
dieDB_ptr _create_dieDB(int lowerLeftX, int lowerLeftY, 
                        int upperRightX, int upperRightY);
void _destroy_dieDB(dieDB_ptr rm_db);
void _place_instance_in_die(dieDB_ptr target_db, masterCellDB_ptr masterCellDB, int is_topDie, instance_ptr target_instance);
void _swap_instance_to_other_die(dieDB_ptr target_db, masterCellDB_ptr masterCellDB, instance_ptr target_instance);
