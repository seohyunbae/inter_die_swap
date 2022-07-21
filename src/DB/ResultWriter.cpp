#include "ResultWriter.h"
#include <iostream>
#include <fstream>

using namespace std;


COORD get_left_lower_coordinate(FCOORD center, int const &sizeX, int const &sizeY)
{
    int llx = 0, lly = 0;
    llx = center.x - sizeX / 2;
    lly = center.y - sizeY / 2;
    COORD ll;
    ll.x = llx, ll.y = lly;
    return ll;
}

void write(dataBase_ptr data, string const &filename)
{
    ofstream fo(filename);

    fo << "TopDiePlacement " << data->dieDB->top_die->numInsts << endl;
    for (int i = 0; i < data->instanceDB->numInsts; ++i)
    {
        instance_ptr inst = data->instanceDB->inst_array[i];
        if (inst->dieNum != top_die_idx)
            continue;
        string name = string(inst->instanceName);
        COORD ll = get_left_lower_coordinate(inst->center, inst->sizeX, inst->sizeY);
        fo << "Inst " << name << " " << ll.x << " " << ll.y << endl;
    }

    fo << "BottomDiePlacement " << data->dieDB->bot_die->numInsts << endl;
    for (int i = 0; i < data->instanceDB->numInsts; ++i)
    {
        instance_ptr inst = data->instanceDB->inst_array[i];
        if (inst->dieNum != bot_die_idx)
            continue;
        string name = string(inst->instanceName);
        COORD ll = get_left_lower_coordinate(inst->center, inst->sizeX, inst->sizeY);
        fo << "Inst " << name << " " << ll.x << " " << ll.y << endl;
    }

    int numTerminals = data->terminalDB->numTerminals;
    fo << "NumTerminals " << numTerminals << endl;
    for (int i = 0; i < numTerminals; ++i)
    {
        terminal_ptr terminal = data->terminalDB->term_array[i];
        string name = terminal->netName;
        FCOORD center = terminal->center;
        fo << "Terminal " << name << " " << (int)center.x << " " << (int)center.y << endl;
    }
}