#pragma once

#include "Tech.h"
#include "Die.h"
#include "Terminal.h"
#include "Instance.h"
#include "Net.h"

#include "global.h"


struct DataBase{
    masterCellDB_ptr    masterCellDB;
    dieDB_ptr           dieDB;
    instanceDB_ptr      instanceDB;
    netDB_ptr           netDB;
    terminalDB_ptr      terminalDB;
}; typedef struct DataBase* dataBase_ptr;


// List of functions
// 1. Construction and destruction of database. 
dataBase_ptr create_DataBase(void);
void destroy_DataBase(dataBase_ptr rm_db);
void construct_MasterCellDB(dataBase_ptr target_db, int numTechs);
void construct_dieDB(dataBase_ptr target_db, int lowerLeftX, int lowerLeftY, int upperRightX, int upperRightY);
void construct_instanceDB(dataBase_ptr target_db, int numInsts);
void construct_netDB(dataBase_ptr target_db, int numNets);
void construct_terminalDB(dataBase_ptr target_db, int sizeX, int sizeY, int spacing);



// 2. MasterCell functions
// A function that creates mastercell and add to database
void add_masterCell(dataBase_ptr target_db, char* masterCellName, char* techName, int numPins, int sizeX, int sizeY);
// A fucntion that creates a pin and link to mastercell. 
void add_pin(dataBase_ptr target_db, char* masterCellName, char* techName, char* pinName, int offsetX, int offsetY);
// A function that returns a pointer of a mastercell. 
masterCell_ptr get_masterCell(dataBase_ptr target_db, char* masterCellName);
// A function that adds a cellspec pointer to mastercell.
void add_cellSpec(dataBase_ptr target_db, char* masterCellName, cellSpec_ptr target_cellSpec);
// A function that returns cellspec with mastercell name and tech name. 
cellSpec_ptr get_cellSpec(dataBase_ptr target_db, char* masterCellName, char* techName);

// 3. Die functions
// A function that places an instance to a die. 
void place_instance_in_die(dataBase_ptr target_db, int is_topDie, instance_ptr target_instance);
// A function that swaps an instance to another die. 
void swap_instance_to_other_die(dataBase_ptr target_db, instance_ptr target_instance);
// A function that creates a database for a die. 
void create_die(dataBase_ptr target_db, int is_top_die,
                char* dieName, char* techName,
                int targetUtil,
                int startX, int startY,
                int rowLength, int rowHeight,
                int repeatCount);

// 4. Instance functions
// A function that creates an instance and adds to built-in database. 
void add_instance(dataBase_ptr target_db, char* instanceName, char* masterCellName);
// A function that returns an instance pointer searched by instance name. 
instance_ptr get_instance_with_name(dataBase_ptr target_db, char* instanceName);
// A function that updates the spec of an instance caused from die swap. 
void update_instance_die_swap(dataBase_ptr target_db, instance_ptr target_instance, char* techName, int dieNum);

// 5. Net functions
// A function that returns a net pointer searched by name. 
net_ptr get_net_with_name(dataBase_ptr target_db, char* netName);
// A function that creates a net and adds to database. 
void add_net(dataBase_ptr target_db, char* netName, int numPins);
// A function that links net and instance. 
void link_net_inst(dataBase_ptr target_db, char* netName, char* instanceName, char* pinName);

// 6. Terminal functions
// A function that creates a terminal and add to terminalDB and correspoinding net. 
void add_terminal(dataBase_ptr target_db, int sizeX, int sizeY, int spacing, net_ptr target_net);
// A function that removes a terminal. Since terminal is repeatedly constructed and destructed, this function is necessary. 
void remove_terminal(dataBase_ptr target_db, terminal_ptr target_term);
// A function that returns a terminal pointer searched by the name of corresponding net. 
terminal_ptr get_terminal(dataBase_ptr target_db, char* netName);
// A function that updates the coordinate of a terminal searched by the name of corresponding net. 
void update_terminal(dataBase_ptr target_db, char* netName, FCOORD coord);
