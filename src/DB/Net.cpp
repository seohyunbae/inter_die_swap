#include "Net.h"

using namespace std;


//Construction and destruction of net data structure
net2inst_ptr _create_Net2Inst(char* netName, char* instanceName, char* pinName)
{
    net2inst_ptr new_link = (net2inst_ptr)malloc(sizeof(struct Net2Inst));
    new_link->netName = strdup(netName);
    new_link->instanceName = strdup(instanceName);
    new_link->pinName = strdup(pinName);
    new_link->next = NULL;
    return new_link;
}


void _destroy_Net2Inst(net2inst_ptr rm_link)
{
    if (!rm_link) return;
    free(rm_link->netName);
    free(rm_link->instanceName);
    free(rm_link->pinName);
    free(rm_link);
}


net_ptr _create_Net(char* netName, int numPins)
{
    net_ptr new_net = (net_ptr)malloc(sizeof(struct Net));
    new_net->netName = strdup(netName);
    new_net->numPins = numPins;

    new_net->numInst = 0;

    new_net->netArrayIndex = 0;
    new_net->cur_numPins = 0;
    new_net->terminal = NULL;
    new_net->instance_head = NULL;
    new_net->pin_array = (net2inst_ptr*)malloc(sizeof(net2inst_ptr) * numPins);
    new_net->next = NULL;

    new_net->pmax_top.x = new_net->pmax_top.y = 0.0;
    new_net->pmin_top.x = new_net->pmin_top.y = 0.0;
    new_net->pmax_bot.x = new_net->pmax_bot.y = 0.0;
    new_net->pmin_top.x = new_net->pmin_bot.y = 0.0;
    return new_net;
}


void _destroy_Net(net_ptr rm_net)
{
    if (!rm_net) return;
    net2inst_ptr sweep = rm_net->instance_head;
    while(sweep)
    {
        net2inst_ptr sweep_next = sweep->next;
        _destroy_Net2Inst(sweep);
        sweep = sweep_next;
    }
    free(rm_net->pin_array);
    free(rm_net->netName);
    free(rm_net);
}


netDB_ptr _create_NetDB(int numNets)
{
    netDB_ptr newDB = (netDB_ptr)malloc(sizeof(struct NetDB));
    newDB->numNets = numNets;
    newDB->cur_bucket = default_hash_size;

    newDB->cur_numNets = 0;
    newDB->net_array = (net_ptr*)calloc(sizeof(net_ptr), numNets);

    newDB->hash_head = (netHash_ptr)calloc(sizeof(struct NetHash), default_hash_size);
    for (int i = 0; i < default_hash_size; i++)
    {
        newDB->hash_head[i].net_head = NULL;
    }
    return newDB;
}


void _destroy_netDB(netDB_ptr rm_db)
{
    if (!rm_db) return;
    for (int i = 0; i < rm_db->cur_bucket; i++)
    {
        net_ptr sweep = rm_db->hash_head[i].net_head;
        while(sweep)
        {
            net_ptr sweep_next = sweep->next;
            _destroy_Net(sweep);
            sweep = sweep_next;
        }
    }
    free(rm_db->hash_head);
    free(rm_db->net_array);
    free(rm_db);
}


//Data Manipulation
void _add_net_inst_link(netDB_ptr target_netDB, instanceDB_ptr target_instDB, net2inst_ptr target_link)
{
    // Place to net hash table first. 
    int hash_index = hash_function(target_netDB->cur_bucket, target_link->netName);
    net_ptr sweep = target_netDB->hash_head[hash_index].net_head;
    while(sweep)
    {
        if (!strcmp(sweep->netName, target_link->netName)) 
        {
            // Currently fron net_ptr
            target_link->next = sweep->instance_head;
            sweep->instance_head = target_link;
            sweep->pin_array[sweep->cur_numPins] = target_link;
            sweep->cur_numPins++;
            
            net2inst_ptr check_inst_repetition = target_link->next;
            while(check_inst_repetition)
            {
                if (!strcmp(check_inst_repetition->instanceName, target_link->instanceName)) break;
                check_inst_repetition = check_inst_repetition->next;
            }
            if (check_inst_repetition == NULL){ // Since there was no matching instance
                sweep->numInst++;
            }

            break;
        }
        sweep = sweep->next;
    }

    // Add instance to net link
    instance_ptr target_instance = _get_instance_with_name(target_instDB, target_link->instanceName);
    inst2net_ptr new_inst_net_link = _create_inst_net_link((void*)sweep, target_link->pinName);
    target_link->to_instLink = new_inst_net_link;
    new_inst_net_link->to_netLink = target_link;
    new_inst_net_link->next = target_instance->net_head;
    new_inst_net_link->net = sweep;
    target_instance->net_head = new_inst_net_link;

    inst2net_ptr check_net_repetition = new_inst_net_link->next;
    net_ptr checkNet = (net_ptr)new_inst_net_link;
    char* checkName = checkNet->netName;
    net_ptr curNet;
    while(check_net_repetition)
    {
        curNet = (net_ptr)check_net_repetition->net;
        if (!strcmp(curNet->netName, checkName)) break;
        check_net_repetition = check_net_repetition->next;
    }
    if (check_net_repetition == NULL){
        target_instance->numNets++;
    }

    target_link->instance = target_instance;
}


void _add_net_to_netDB(netDB_ptr target_db, net_ptr target_net)
{
    target_db->net_array[target_db->cur_numNets] = target_net;
    target_net->netArrayIndex = target_db->cur_numNets;
    target_db->cur_numNets++;
    int hash_index = hash_function(target_db->cur_bucket, target_net->netName);
    target_net->next = target_db->hash_head[hash_index].net_head;
    target_db->hash_head[hash_index].net_head = target_net;
}


net_ptr _get_net_with_name(netDB_ptr target_db, char* netName)
{
    int hash_index = hash_function(target_db->cur_bucket, netName);
    net_ptr sweep = target_db->hash_head[hash_index].net_head;
    while(sweep)
    {
        if (!strcmp(sweep->netName, netName)) return sweep;
        sweep = sweep->next;
    }
    return NULL;
}


void _print_all_nets(netDB_ptr target_db)
{
    for (int i = 0; i < target_db->cur_numNets; i++)
    {
        printf("Net Name = %s\n", target_db->net_array[i]->netName);
    }
}
