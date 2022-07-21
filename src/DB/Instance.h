#pragma once

#include "global.h"
#include "Tech.h"

//for die partition
// #include "Die.h"


struct Inst2Net{
    void* net;
    void* to_netLink;
    char* pinName;
    FCOORD curCoord;
    FCOORD curOffsetCoord;
    FCOORD topOffsetCoord;
    FCOORD botOffsetCoord;
    struct Inst2Net* next;
}; typedef struct Inst2Net* inst2net_ptr;


struct Instance{
    // from input file
    char* instanceName;
    char* masterCellName;

    FCOORD center;
    int instIndex;
    int sizeX;
    int sizeY;
    int area;

    int numNets;

    int dieNum; // bottom die -> 1, top die -> 0 (for code extension)
    int intArrayIndex;
    struct Instance* die_next;
    inst2net_ptr net_head;
    masterCell_ptr masterCell;
    cellSpec_ptr curTech;
    struct Instance* next;

    //for die partition
    bool update;
    int gain;
    bool fixed;
    int bestDie;
    std::list<struct Instance*>::iterator GB_itr; //inst's position in gain bucket

    struct POS binCoord;
    struct POS fft_coord_min, fft_coord_max;
    struct FPOS charge_grad;
    struct FPOS wlen_grad;
    struct FPOS grad;
}; typedef struct Instance* instance_ptr;


struct InstanceHash{
    instance_ptr instance_head;
}; typedef struct InstanceHash* instanceHash_ptr;


struct InstanceDB{
    int cur_bucket;
    int numInsts;
    int cur_numInsts;
    instance_ptr* inst_array;
    instanceHash_ptr hash_head;
}; typedef struct InstanceDB* instanceDB_ptr;


//List of functions
inst2net_ptr _create_inst_net_link(void* net, char* pinName);
instance_ptr _create_instance(char* instanceName, char* masterCellName, masterCellDB_ptr target_db);
void _destroy_instance(instance_ptr rm_instance);
instanceDB_ptr _create_instanceDB(int numInsts);
void _destroy_instanceDB(instanceDB_ptr rm_db);

void _add_instance(instanceDB_ptr target_db, instance_ptr target_instance);
instance_ptr _get_instance_with_name(instanceDB_ptr target_db, char* instanceName);
void _update_instance(masterCellDB_ptr MasterCellDB, instance_ptr target_instance, char* techName, int dieNum);
void _print_all_instance(instanceDB_ptr target_db);