#include "Die.h"

using namespace std;


die_ptr _create_die(char* dieName, char* techName,
                    int lowerLeftX, int lowerLeftY,
                    int upperRightX, int upperRightY,
                    int targetUtil,
                    int startX, int startY,
                    int rowLength, int rowHeight,
                    int repeatCount)
{
    die_ptr new_die = (die_ptr)malloc(sizeof(struct Die));
    new_die->dieName = strdup(dieName);
    new_die->techName = strdup(techName);
    new_die->lowerLeftX = lowerLeftX;
    new_die->lowerLeftY = lowerLeftY;
    new_die->upperRightX = upperRightX;
    new_die->upperRightY = upperRightY;
    new_die->targetUtil = targetUtil;
    new_die->startX = startX;
    new_die->startY = startY;
    // shchung
    new_die->rowLength = rowLength;
    new_die->rowHeight = rowHeight;
    new_die->repeatCount = repeatCount;

    new_die->dieArea = (upperRightX - lowerLeftX) * (upperRightY - lowerLeftY);

    new_die->cur_bucket = default_hash_size;
    new_die->numInsts = 0;
    new_die->curArea = 0;
    new_die->curUtil = 0;
    new_die->hash_head = (dieInstanceHash_ptr)calloc(sizeof(struct DieInstanceHash), default_hash_size);
    for (int i = 0; i < default_hash_size; i++)
    {
        new_die->hash_head[i].dieInstance_head = NULL;
    }
    return new_die;
}


void _destroy_die(die_ptr rm_die)
{
    if (!rm_die) return;
    free(rm_die->dieName);
    free(rm_die->techName);
    free(rm_die->hash_head);
    free(rm_die);
}


dieDB_ptr _create_dieDB(int lowerLeftX, int lowerLeftY, 
                       int upperRightX, int upperRightY)
{
    dieDB_ptr new_dieDB = (dieDB_ptr)malloc(sizeof(struct DieDB));
    new_dieDB->lowerLeftX = lowerLeftX;
    new_dieDB->lowerLeftY = lowerLeftY;
    new_dieDB->upperRightX = upperRightX;
    new_dieDB->upperRightY = upperRightY;
    new_dieDB->centerX = ((double)upperRightX - (double)lowerLeftX) / 2;
    new_dieDB->centerY = ((double)upperRightY - (double)lowerLeftY) / 2;
    new_dieDB->dieArea = (upperRightX - lowerLeftX) * (upperRightY - lowerLeftY);

    new_dieDB->top_die = NULL;
    new_dieDB->bot_die = NULL;
    return new_dieDB;
}


void _destroy_dieDB(dieDB_ptr rm_db)
{
    if (!rm_db) return;
    _destroy_die(rm_db->top_die);
    _destroy_die(rm_db->bot_die);
    free(rm_db);
}


void _place_instance_in_die(dieDB_ptr target_db, masterCellDB_ptr masterCellDB, int is_topDie, instance_ptr target_instance)
{
    if(is_topDie)
    {
        int hash_index = hash_function(target_db->top_die->cur_bucket, target_instance->instanceName);
        target_instance->die_next = target_db->top_die->hash_head[hash_index].dieInstance_head;
        target_db->top_die->hash_head[hash_index].dieInstance_head = target_instance;

        _update_instance(masterCellDB, target_instance, target_db->top_die->techName, 0);
        ///////
        target_db->top_die->numInsts += 1;
        ///////
        target_db->top_die->curArea += target_instance->area;
        target_db->top_die->curUtil = (double)target_db->top_die->curArea / (double)target_db->dieArea;
    }
    else
    {
        int hash_index = hash_function(target_db->bot_die->cur_bucket, target_instance->instanceName);
        target_instance->die_next = target_db->bot_die->hash_head[hash_index].dieInstance_head;
        target_db->bot_die->hash_head[hash_index].dieInstance_head = target_instance;

        _update_instance(masterCellDB, target_instance, target_db->bot_die->techName, 1);
        ///////
        target_db->bot_die->numInsts += 1;
        ///////
        target_db->bot_die->curArea += target_instance->area;
        target_db->bot_die->curUtil = (double)target_db->bot_die->curArea / (double)target_db->dieArea;
    }
}


void _swap_instance_to_other_die(dieDB_ptr target_db, masterCellDB_ptr masterCellDB, instance_ptr target_instance)
{
    if(target_instance->dieNum == 0) //Top die
    {
        int hash_index = hash_function(target_db->top_die->cur_bucket, target_instance->instanceName);
        instance_ptr sweep = target_db->top_die->hash_head[hash_index].dieInstance_head;
        if (sweep == target_instance)
        {
            target_db->top_die->hash_head[hash_index].dieInstance_head = sweep->die_next;
        }
        else
        {
            while(sweep)
            {
                if (sweep->die_next == target_instance)
                {
                    sweep->die_next = sweep->die_next->die_next;
                    break;
                }
                sweep = sweep->die_next;
            }
        }
        ///////
        target_db->top_die->numInsts -= 1;
        ///////
        target_db->top_die->curArea -= target_instance->area;
        target_db->top_die->curUtil = (double)target_db->top_die->curArea / (double)target_db->dieArea;
        _place_instance_in_die(target_db, masterCellDB, target_instance->dieNum, target_instance);
    }
    else
    {
        int hash_index = hash_function(target_db->bot_die->cur_bucket, target_instance->instanceName);
        instance_ptr sweep = target_db->bot_die->hash_head[hash_index].dieInstance_head;
        if (sweep == target_instance)
        {
            target_db->bot_die->hash_head[hash_index].dieInstance_head = sweep->die_next;
        }
        else
        {
            while(sweep)
            {
                if (sweep->die_next == target_instance)
                {
                    sweep->die_next = sweep->die_next->die_next;
                    break;
                }
                sweep = sweep->die_next;
            }
        }
        ///////
        target_db->bot_die->numInsts -= 1;
        ///////
        target_db->bot_die->curArea -= target_instance->area;
        target_db->bot_die->curUtil = (double)target_db->bot_die->curArea / (double)target_db->dieArea;
        _place_instance_in_die(target_db, masterCellDB, target_instance->dieNum, target_instance);
    }
    // printf("Instname: %s\ncurDie: %d\ncurSize: (%d, %d)\n", target_instance->instanceName, target_instance->dieNum, target_instance->sizeX, target_instance->sizeY);
}