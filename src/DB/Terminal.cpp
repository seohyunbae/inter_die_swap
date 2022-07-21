#include "Terminal.h"

using namespace std;


//Construction and destruction of data structure. 
terminal_ptr _create_terminal(int sizeX, int sizeY, int spacing, net_ptr target_net)
{
    terminal_ptr new_term = (terminal_ptr)malloc(sizeof(struct Terminal));
    new_term->sizeX = sizeX;
    new_term->sizeY = sizeY;
    new_term->spacing = spacing;
    new_term->center.x = 0.0;
    new_term->center.y = 0.0;
    new_term->netName = strdup(target_net->netName);
    new_term->term_net = target_net;
    target_net->terminal = new_term;
    new_term->next = NULL;
    return new_term;
}


void _destroy_terminal(terminal_ptr rm_term)
{
    if (!rm_term) return;
    free(rm_term->netName);
    free(rm_term);
}


terminalDB_ptr _create_terminalDB(int sizeX, int sizeY, int spacing)
{
    terminalDB_ptr new_db = (terminalDB_ptr)malloc(sizeof(struct TerminalDB));
    new_db->numTerminals = 0;
    new_db->cur_bucket = default_hash_size;
    new_db->sizeX = sizeX;
    new_db->sizeY = sizeY;
    new_db->spacing = spacing;
    new_db->area = sizeX * sizeY;
    
    new_db->term_array = NULL;
    new_db->sizeX_w_spacing = sizeX + spacing;
    new_db->sizeY_w_spacing = sizeY + spacing;
    new_db->area_w_spacing = (new_db->sizeX_w_spacing) * (new_db->sizeY_w_spacing);

    new_db->hash_head = (terminalHash_ptr)calloc(sizeof(struct TerminalDB), default_hash_size);
    for (int i = 0; i < default_hash_size; i++)
    {
        new_db->hash_head[i].terminal_head = NULL;
    }
    return new_db;
}


void _destroy_terminalDB(terminalDB_ptr rm_db)
{
    if (!rm_db) return;
    for (int i = 0; i < rm_db->cur_bucket; i++)
    {
        terminal_ptr sweep = rm_db->hash_head[i].terminal_head;
        while(sweep)
        {
            terminal_ptr sweep_next = sweep->next;
            _destroy_terminal(sweep);
            sweep = sweep_next;
        }
    }
    free(rm_db->hash_head);
    if (rm_db->term_array)
    {
        free(rm_db->term_array);
    }
    free(rm_db);
}


//Data manipulation
void _add_terminal(terminalDB_ptr target_db, terminal_ptr target_term)
{
    int hash_index = hash_function(target_db->cur_bucket, target_term->netName);
    target_term->next = target_db->hash_head[hash_index].terminal_head;
    target_db->hash_head[hash_index].terminal_head = target_term;
    target_term->termIndex = target_db->numTerminals;
    target_db->numTerminals++;

    //Add to terminal array;
    target_db->term_array = (terminal_ptr*)realloc(target_db->term_array, sizeof(terminal_ptr)*target_db->numTerminals);
    target_db->term_array[target_db->numTerminals-1] = target_term;
}


void _remove_terminal(terminalDB_ptr target_db, terminal_ptr target_term)
{
    int hash_index = hash_function(target_db->cur_bucket, target_term->netName);
    terminal_ptr sweep;
    for (int i = 0; i < target_db->numTerminals; i++)
    {
        sweep = target_db->term_array[i];
        if (sweep == target_term)
        {
            for(int j = i + 1; j < target_db->numTerminals; j++)
            {
                target_db->term_array[j]->termIndex--;
                target_db->term_array[j-1] = target_db->term_array[j];
            }
        }
    }

    sweep = target_db->hash_head[hash_index].terminal_head;
    if (sweep == target_term) // When first term is the target
    {
        target_db->hash_head[hash_index].terminal_head = target_term->next;
        _destroy_terminal(target_term);
        target_db->numTerminals--;
        return;
    }
    while(sweep)
    {
        if(sweep->next == target_term)
        {
            sweep->next = target_term->next;
            _destroy_terminal(target_term);
            target_db->numTerminals--;
            return;
        }
        sweep = sweep->next;
    }
}


terminal_ptr _get_terminal(terminalDB_ptr target_db, char* netName)
{
    int hash_index = hash_function(target_db->cur_bucket, netName);
    terminal_ptr sweep = target_db->hash_head[hash_index].terminal_head;
    while(sweep)
    {
        if (!strcmp(sweep->netName, netName)) return sweep;
        sweep = sweep->next;
    }
    return NULL;
}


void _update_terminal(terminalDB_ptr target_db, char* netName, FCOORD coord)
{
    terminal_ptr target_term = _get_terminal(target_db, netName);
    if (!target_term) return;
    
    target_term->center.x = coord.x;
    target_term->center.y = coord.y;
}