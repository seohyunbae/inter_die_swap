#include "DataBase.h"

using namespace std;



dataBase_ptr create_DataBase(void)
{
    dataBase_ptr new_db = (dataBase_ptr)malloc(sizeof(struct DataBase));
    return new_db;
}

void print_all_data(dataBase_ptr db);


void destroy_DataBase(dataBase_ptr rm_db)
{
    _destroy_masterCellDB(rm_db->masterCellDB);
    _destroy_dieDB(rm_db->dieDB);
    _destroy_instanceDB(rm_db->instanceDB);
    _destroy_netDB(rm_db->netDB);
    _destroy_terminalDB(rm_db->terminalDB);
    free(rm_db);
}

// Database construction
void construct_MasterCellDB(dataBase_ptr target_db, int numTechs)
{
    target_db->masterCellDB= _create_masterCellDB(numTechs);
}


void construct_dieDB(dataBase_ptr target_db, int lowerLeftX, int lowerLeftY, int upperRightX, int upperRightY)
{
    target_db->dieDB = _create_dieDB(lowerLeftX, lowerLeftY, upperRightX, upperRightY);
}


void construct_instanceDB(dataBase_ptr target_db, int numInsts)
{
    target_db->instanceDB = _create_instanceDB(numInsts);
}


void construct_netDB(dataBase_ptr target_db, int numNets)
{
    target_db->netDB = _create_NetDB(numNets);
}


void construct_terminalDB(dataBase_ptr target_db, int sizeX, int sizeY, int spacing)
{
    target_db->terminalDB = _create_terminalDB(sizeX, sizeY, spacing);
}



// MasterCell functions
void add_masterCell(dataBase_ptr target_db, char* masterCellName, char* techName, int numPins, int sizeX, int sizeY)
{
    masterCell_ptr target_mc = _get_masterCell(target_db->masterCellDB, masterCellName);
    if (target_mc == NULL) // First tech. Master Cell node is not created yet. 
    {
        masterCell_ptr new_mc   = _create_masterCell(masterCellName);
        _add_masterCell(target_db->masterCellDB, new_mc);
    }
    cellSpec_ptr new_cs     = _create_cellSpec(masterCellName, techName, numPins, sizeX, sizeY);
    _add_cellSpec(target_db->masterCellDB, masterCellName, new_cs);
}

void add_pin(dataBase_ptr target_db, char* masterCellName, char* techName, char* pinName, int offsetX, int offsetY)
{
    pin_ptr new_pin = _create_pin(pinName, offsetX, offsetY);
    _add_pin(target_db->masterCellDB, masterCellName, techName, new_pin);
}

masterCell_ptr get_masterCell(dataBase_ptr target_db, char* masterCellName)
{
    return _get_masterCell(target_db->masterCellDB, masterCellName);
}

void add_cellSpec(dataBase_ptr target_db, char* masterCellName, cellSpec_ptr target_cellSpec)
{
    _add_cellSpec(target_db->masterCellDB, masterCellName, target_cellSpec);
}

cellSpec_ptr get_cellSpec(dataBase_ptr target_db, char* masterCellName, char* techName)
{
    return _get_cellSpec(target_db->masterCellDB, masterCellName, techName);
}

// Die functions
void place_instance_in_die(dataBase_ptr target_db, int is_topDie, instance_ptr target_instance)
{
    _place_instance_in_die(target_db->dieDB, target_db->masterCellDB, is_topDie, target_instance);
}

void swap_instance_to_other_die(dataBase_ptr target_db, instance_ptr target_instance)
{
    _swap_instance_to_other_die(target_db->dieDB, target_db->masterCellDB, target_instance);
}

void create_die(dataBase_ptr target_db, int is_top_die,
                char* dieName, char* techName,
                int targetUtil,
                int startX, int startY,
                int rowLength, int rowHeight,
                int repeatCount)
{
    die_ptr new_die = _create_die(dieName, techName,
                                  target_db->dieDB->lowerLeftX, target_db->dieDB->lowerLeftY,
                                  target_db->dieDB->upperRightX, target_db->dieDB->upperRightY,
                                  targetUtil,
                                  startX, startY,
                                  rowLength, rowHeight,
                                  repeatCount);
    if (is_top_die) target_db->dieDB->top_die = new_die;
    else target_db->dieDB->bot_die = new_die;
}


// Instance functions
void add_instance(dataBase_ptr target_db, char* instanceName, char* masterCellName)
{
    instance_ptr new_inst = _create_instance(instanceName, masterCellName, target_db->masterCellDB);
    _add_instance(target_db->instanceDB, new_inst);
}

instance_ptr get_instance_with_name(dataBase_ptr target_db, char* instanceName)
{
    return _get_instance_with_name(target_db->instanceDB, instanceName);
}

void update_instance_die_swap(dataBase_ptr target_db, instance_ptr target_instance, char* techName, int dieNum)
{
    _update_instance(target_db->masterCellDB, target_instance, techName, dieNum);
}


// Net functions
net_ptr get_net_with_name(dataBase_ptr target_db, char* netName)
{
    return _get_net_with_name(target_db->netDB, netName);
}

void add_net(dataBase_ptr target_db, char* netName, int numPins)
{
    net_ptr new_net = _create_Net(netName, numPins);
    _add_net_to_netDB(target_db->netDB, new_net);
}

void link_net_inst(dataBase_ptr target_db, char* netName, char* instanceName, char* pinName)
{
    net2inst_ptr net2inst_link = _create_Net2Inst(netName, instanceName, pinName);
    _add_net_inst_link(target_db->netDB, target_db->instanceDB, net2inst_link);
}


// Terminal function
void add_terminal(dataBase_ptr target_db, int sizeX, int sizeY, int spacing, net_ptr target_net)
{
    terminal_ptr new_term = _create_terminal(sizeX, sizeY, spacing, target_net);
    _add_terminal(target_db->terminalDB, new_term);
}

void remove_terminal(dataBase_ptr target_db, terminal_ptr target_term)
{
    _remove_terminal(target_db->terminalDB, target_term);
}

terminal_ptr get_terminal(dataBase_ptr target_db, char* netName)
{
    return _get_terminal(target_db->terminalDB, netName);
}

void update_terminal(dataBase_ptr target_db, char* netName, FCOORD coord)
{
    _update_terminal(target_db->terminalDB, netName, coord);
}
