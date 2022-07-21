#include "Tech.h"

using namespace std;


//Data construction with memory allocation
pin_ptr _create_pin(char* pinName, int offsetX, int offsetY)
{
    pin_ptr new_pin = (pin_ptr)malloc(sizeof(struct Pin));
    new_pin->pinName = strdup(pinName);
    new_pin->offsetX = (double)offsetX;
    new_pin->offsetY = (double)offsetY;
    new_pin->next = NULL;
    return new_pin;
}


void _destroy_pin(pin_ptr rm_pin)
{
    if (!rm_pin) return;
    free(rm_pin->pinName);
    free(rm_pin);
}


cellSpec_ptr _create_cellSpec(char* masterCellName, char* techName, int numPins, int sizeX, int sizeY)
{
    cellSpec_ptr new_cellspec = (cellSpec_ptr)malloc(sizeof(struct CellSpec));
    new_cellspec->masterCellName = strdup(masterCellName);
    new_cellspec->techName = strdup(techName);
    new_cellspec->numPins = numPins;
    new_cellspec->sizeX = sizeX;
    new_cellspec->sizeY = sizeY;
    new_cellspec->pin_head = NULL;
    new_cellspec->next = NULL;
    return new_cellspec;
}


void _destroy_cellSpec(cellSpec_ptr rm_cellSpec)
{
    if (!rm_cellSpec) return;
    free(rm_cellSpec->masterCellName);
    free(rm_cellSpec->techName);
    pin_ptr sweep = rm_cellSpec->pin_head;
    while(sweep)
    {
        pin_ptr sweep_next = sweep->next;
        _destroy_pin(sweep);
        sweep = sweep_next;
    }
    free(rm_cellSpec);
}


masterCell_ptr _create_masterCell(char* masterCellName)
{
    masterCell_ptr new_mc = (masterCell_ptr)malloc(sizeof(struct MasterCell));
    new_mc->masterCellName = strdup(masterCellName);
    new_mc->cellSpec_head = NULL;
    new_mc->next = NULL;
    return new_mc;
}


void _destroy_masterCell(masterCell_ptr rm_mc)
{
    if (!rm_mc) return;
    free(rm_mc->masterCellName);
    cellSpec_ptr sweep = rm_mc->cellSpec_head;
    while(sweep)
    {
        cellSpec_ptr sweep_next = sweep->next;
        _destroy_cellSpec(sweep);
        sweep = sweep_next;
    }
    free(rm_mc);
}


masterCellDB_ptr _create_masterCellDB(int numTechs)
{
    masterCellDB_ptr new_db = (masterCellDB_ptr)malloc(sizeof(struct MasterCellDB));
    new_db->numTechs = numTechs;
    new_db->cur_bucket = default_hash_size;
    new_db->num_mastercell = 0;
    new_db->hash_head = (masterCellHash_ptr)calloc(sizeof(struct MasterCellHash), default_hash_size);
    for (int i = 0; i < default_hash_size; i++)
    {
        new_db->hash_head[i].masterCell_head = NULL;
    }
    return new_db;
}


void _destroy_masterCellDB(masterCellDB_ptr rm_db)
{
    if (!rm_db) return;
    for (int i = 0; i < rm_db->cur_bucket; i++)
    {
        masterCell_ptr sweep = rm_db->hash_head[i].masterCell_head;
        while (sweep)
        {
            masterCell_ptr sweep_next = sweep->next;
            _destroy_masterCell(sweep);
            sweep = sweep_next;
        }
    }
    free(rm_db->hash_head);
    free(rm_db);
}


//Data manipulation
void _add_masterCell(masterCellDB_ptr target_db, masterCell_ptr target_masterCell)
{
    int hash_index = hash_function(target_db->cur_bucket, target_masterCell->masterCellName);

    target_masterCell->next = target_db->hash_head[hash_index].masterCell_head;
    target_db->hash_head[hash_index].masterCell_head = target_masterCell;
    target_db->num_mastercell++;
}


masterCell_ptr _get_masterCell(masterCellDB_ptr target_db, char* masterCellName)
{
    int hash_index = hash_function(target_db->cur_bucket, masterCellName);
    masterCell_ptr sweep_mc = target_db->hash_head[hash_index].masterCell_head;
    while(sweep_mc)
    {
        if(!strcmp(masterCellName, sweep_mc->masterCellName))
        {
            return sweep_mc;
        }
        sweep_mc = sweep_mc->next;   
    }
    return NULL;
}


void _add_cellSpec(masterCellDB_ptr target_db, char* masterCellName, cellSpec_ptr target_cellSpec)
{
    masterCell_ptr target_mc = _get_masterCell(target_db, masterCellName);
    target_cellSpec->next = target_mc->cellSpec_head;
    target_mc->cellSpec_head = target_cellSpec;
}


cellSpec_ptr _get_cellSpec(masterCellDB_ptr target_db, char* masterCellName, char* techName)
{
    masterCell_ptr target_mc = _get_masterCell(target_db, masterCellName);
    cellSpec_ptr sweep_cs = target_mc->cellSpec_head;
    while(sweep_cs)
    {
        if (!strcmp(techName, sweep_cs->techName)) return sweep_cs;
        sweep_cs = sweep_cs->next;
    }
    return NULL;
}


void _add_pin(masterCellDB_ptr target_db, char* masterCellName, char* techName, pin_ptr target_pin)
{
    cellSpec_ptr target_cellSpec = _get_cellSpec(target_db, masterCellName, techName);
    target_pin->next = target_cellSpec->pin_head;
    target_cellSpec->pin_head = target_pin;
}