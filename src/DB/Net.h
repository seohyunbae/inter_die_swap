#pragma once

#include "global.h"
#include "Instance.h"


struct Net2Inst{
    instance_ptr instance;
    inst2net_ptr to_instLink;
    char* netName;
    char* instanceName;
    char* pinName;
    FCOORD curCoord;
    struct Net2Inst* next;

    // For global placement
    struct FPOS e1;
    struct FPOS e2;
    
    struct POS flg1;
    struct POS flg2;
}; typedef struct Net2Inst* net2inst_ptr;


struct Net{
    char* netName;
    int numPins;

    int netArrayIndex;
    int numInst; // Number of Nets that it's included
    void* terminal;
    unsigned int cur_numPins;
    net2inst_ptr instance_head;
    net2inst_ptr* pin_array;
    struct Net* next;

    //for die partition
    bool update;
    bool cut;
    int instance_distribution[2]; //key: top-0, bot-1;

    //For global placement and HPWL calculation
    struct FPOS pmax_top;
    struct FPOS pmin_top;
    struct FPOS pmax_bot;
    struct FPOS pmin_bot;

    struct FPOS sum_num1_top;
    struct FPOS sum_num1_bot;
    struct FPOS sum_num2_top;
    struct FPOS sum_num2_bot;

    struct FPOS sum_denom1_top;
    struct FPOS sum_denom1_bot;
    struct FPOS sum_denom2_top;
    struct FPOS sum_denom2_bot;
    
}; typedef struct Net* net_ptr;


struct NetHash{
    net_ptr net_head;
}; typedef struct NetHash* netHash_ptr;


struct NetDB{
    int numNets;
    int cur_bucket;
    int cur_numNets;
    net_ptr* net_array;
    netHash_ptr hash_head;
}; typedef struct NetDB* netDB_ptr;


net2inst_ptr _create_Net2Inst(char* netName, char* instanceName, char* pinName);
void _destroy_Net2Inst(net2inst_ptr rm_link);
net_ptr _create_Net(char* netName, int numPins);
void _destroy_Net(net_ptr rm_net);
netDB_ptr _create_NetDB(int numNets);
void _destroy_netDB(netDB_ptr rm_db);


void _add_net_inst_link(netDB_ptr target_netDB, instanceDB_ptr target_instDB, net2inst_ptr target_link);
void _add_net_to_netDB(netDB_ptr target_db, net_ptr target_net);
net_ptr _get_net_with_name(netDB_ptr target_db, char* netName);
void _print_all_nets(netDB_ptr target_db);
