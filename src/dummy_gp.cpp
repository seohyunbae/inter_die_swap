#include "FM/FM_Bin.h"
#include "DB/InputParser.h"

namespace FM_Die{

void dummy_gp(dataBase_ptr db){
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> dis(0.0, 1.0);

    double x_length = (double)db->dieDB->upperRightX - (double)db->dieDB->lowerLeftX;
    double y_length = (double)db->dieDB->upperRightY - (double)db->dieDB->lowerLeftY;

    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        inst->center.x = x_length * dis(gen);
        inst->center.y = y_length * dis(gen);
        inst->dieNum = 0;
    }

}


}