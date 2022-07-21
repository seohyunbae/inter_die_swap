#include "Instance.h"


//Creation and deletion of database. 
inst2net_ptr _create_inst_net_link(void* net, char* pinName)
{
    inst2net_ptr new_link = (inst2net_ptr)malloc(sizeof(struct Inst2Net));
    new_link->net = net;
    new_link->pinName = strdup(pinName);
    new_link->next = NULL;
    return new_link;
}


instance_ptr _create_instance(char* instanceName, char* masterCellName, masterCellDB_ptr target_db)
{
    instance_ptr new_inst = (instance_ptr)malloc(sizeof(struct Instance));
    new_inst->instanceName = strdup(instanceName);
    new_inst->masterCellName = strdup(masterCellName);

    new_inst->numNets = 0;

    new_inst->net_head = NULL;
    new_inst->intArrayIndex = 0;
    new_inst->masterCell = _get_masterCell(target_db, masterCellName);
    new_inst->next = NULL;
    return new_inst;
}


void _destroy_instance(instance_ptr rm_instance)
{
    if (!rm_instance) return;
    free(rm_instance->instanceName);
    free(rm_instance->masterCellName);
    inst2net_ptr sweep_link = rm_instance->net_head;
    while(sweep_link)
    {
        inst2net_ptr sweep_next = sweep_link->next;
        free(sweep_link->pinName);
        free(sweep_link);
        sweep_link = sweep_next;
    }
    free(rm_instance);
}


instanceDB_ptr _create_instanceDB(int numInsts)
{
    instanceDB_ptr new_db = (instanceDB_ptr)malloc(sizeof(struct InstanceDB));
    new_db->cur_bucket = default_hash_size;
    new_db->numInsts = numInsts;
    new_db->cur_numInsts = 0;
    new_db->inst_array = (instance_ptr*)calloc(sizeof(instance_ptr), numInsts);
    new_db->hash_head = (instanceHash_ptr)calloc(sizeof(struct InstanceHash), default_hash_size);
    for (int i = 0; i < default_hash_size; i++)
    {
        new_db->hash_head[i].instance_head = NULL;
    }
    return new_db;
}


void _destroy_instanceDB(instanceDB_ptr rm_db)
{
    if (!rm_db) return;
    for (int i = 0; i < rm_db->cur_bucket; i++)
    {
        instance_ptr sweep = rm_db->hash_head[i].instance_head;
        while(sweep)
        {
            instance_ptr sweep_next = sweep->next;
            _destroy_instance(sweep);
            sweep = sweep_next;
        }
    }
    free(rm_db->inst_array);
    free(rm_db->hash_head);
    free(rm_db);
}


//Data Manipulation
void _add_instance(instanceDB_ptr target_db, instance_ptr target_instance)
{
    target_db->inst_array[target_db->cur_numInsts] = target_instance;
    target_instance->intArrayIndex = target_db->cur_numInsts;
    target_instance->instIndex = target_db->cur_numInsts;
    target_db->cur_numInsts++;

    int hash_index = hash_function(target_db->cur_bucket, target_instance->instanceName);
    target_instance->next = target_db->hash_head[hash_index].instance_head;
    target_db->hash_head[hash_index].instance_head = target_instance;
}


instance_ptr _get_instance_with_name(instanceDB_ptr target_db, char* instanceName)
{
    int hash_index = hash_function(target_db->cur_bucket, instanceName);
    instance_ptr sweep_inst = target_db->hash_head[hash_index].instance_head;
    while(sweep_inst)
    {
        if(!strcmp(sweep_inst->instanceName, instanceName)) return sweep_inst;
        sweep_inst = sweep_inst->next;
    }
    return NULL;
}


void _update_instance(masterCellDB_ptr MasterCellDB, instance_ptr target_instance, char* techName, int dieNum)
{
    cellSpec_ptr target_tech = _get_cellSpec(MasterCellDB, target_instance->masterCellName, techName);
    target_instance->sizeX = target_tech->sizeX;
    target_instance->sizeY = target_tech->sizeY;
    target_instance->area = target_instance->sizeX * target_instance->sizeY;
    target_instance->curTech = target_tech;
    target_instance->dieNum = dieNum;
}


void _print_all_instance(instanceDB_ptr target_db)
{
    for (int i = 0; i < target_db->cur_numInsts; i++)
    {
        printf("Instname = %s\n", target_db->inst_array[i]->instanceName);
    }
}