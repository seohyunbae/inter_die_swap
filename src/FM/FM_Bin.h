#ifndef _FM_BIN_
#define _FM_BIN_

#include "FM_Die.h"

namespace FM_Die{

class Bin{
    public:
        //coordinate 기준은 lower left point
        int bin_idx;
        double size_x;
        double size_y;
        double coord_x;
        double coord_y;

        int die_num; //0: top, 1:bot

        int inst_num;
        double sum_inst_area;

        double bin_area;
        double cur_util;
        double overflow;

        std::vector<instance_ptr> inst_vec;

        Bin();
        Bin(Bin* bin); //top bin복사해서 bot bin 만들어줌
};

class AllBin{
    public:
        std::vector<Bin*> top_bin_vec;
        std::vector<Bin*> bot_bin_vec;

        int x_bin_num;
        int y_bin_num;
        double die_size_x;
        double die_size_y;
        double bin_size_x;
        double bin_size_y;
        double target_util[2]; //0:top, 1:bot
        int bin_num;

        double ovf_sum[2]; //sum of overflow
        double ovf_avg[2]; //overflow average, 0:top, 1:bot

        AllBin(int x_num, int y_num, dataBase_ptr db);
        ~AllBin();
};

//처음으로 bin 내부의 instance를 따지는 부분
//true면 top die에서 시작, false면 bot die에서 시작
bool first_count_inst(instance_ptr inst, AllBin& allbin, dataBase_ptr db);
//util, overflow 계산 및 제대로 bin에 반영되어 있는지 test (가장 처음)
void first_cal_overflow(AllBin& allbin, dataBase_ptr db);

//이후 instance가 움직일 때마다 bin과 instance update하는 부분
void inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db);

}
#endif