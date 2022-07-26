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
        double bin_area;

        int die_num; //0: top, 1:bot

        int inst_num;
        unsigned int sum_inst_area;
        double cur_util;
        double overflow; //%단위
        std::list<instance_ptr> inst_list;

        Bin(): sum_inst_area(0), die_num(0), inst_num(0) {};
        Bin(Bin* bin); //top bin복사해서 bot bin 만들어줌
};

class AllBin{
    public:
        std::vector<Bin*> top_bin_vec;
        std::vector<Bin*> bot_bin_vec;

        std::map<int, std::list<Bin*>*> bin_bucket;

        int x_bin_num;
        int y_bin_num;
        double die_size_x;
        double die_size_y;
        double bin_size_x;
        double bin_size_y;
        double target_util[2]; //0:top, 1:bot
        int bin_num;

        //overflow는 %댠위
        double ovf_sum[2]; //sum of overflow
        double ovf_avg[2]; //overflow average, 0:top, 1:bot

        AllBin(int x_num, int y_num, dataBase_ptr db);
        ~AllBin();
};

//처음으로 bin 내부의 instance를 따지는 부분
//true면 top die에서 시작, false면 bot die에서 시작
void first_count_inst(instance_ptr inst, AllBin& allbin, dataBase_ptr db);
//util, overflow 계산 및 제대로 bin에 반영되어 있는지 test (가장 처음)
void first_cal_overflow(AllBin& allbin, dataBase_ptr db);
//
void init_bin(AllBin& allbin, dataBase_ptr db);

//이후 instance가 움직일 때마다 bin과 instance update하는 부분
//bin bucket update도 여기서 한번에 이루어진다.
void before_inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db);
void after_inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db);
void ovf_mv_and_up(instance_ptr inst, AllBin& allbin, dataBase_ptr db, GainBucket& gb);

void bin_for_best_die_move(instance_ptr inst, dataBase_ptr db, GainBucket& gb, AllBin& allbin);
bool bin_for_best_reset(dataBase_ptr db, GainBucket& gb, AllBin& allbin);

//init fm
bool bin_init_condition(dataBase_ptr db, int die_num);
void bin_init_db_parameter(dataBase_ptr db, GainBucket& gb, int die_num);
void bin_init_partition(dataBase_ptr db, GainBucket& gb, int die_num);

//overflow based fm
void make_bin_bucket(AllBin& allbin, dataBase_ptr db);
bool overflow_condition(AllBin& allbin, double base_overflow);
instance_ptr find_basecell(AllBin& allbin, Bin* bin, double area_diff, dataBase_ptr db);
instance_ptr overflow_basecell(AllBin& allbin, dataBase_ptr db, Bin* bin);
void overflow_fm(AllBin& allbin, dataBase_ptr db, GainBucket& gb, double base_overflow);

//bi partition-bin based
void bin_mv_and_up(instance_ptr inst, AllBin& allbin, dataBase_ptr db, GainBucket& gb);
bool bin_basecell(instance_ptr inst, dataBase_ptr db, AllBin& allbin, double base_overflow);
void bin_bi_partition(AllBin& allbin, dataBase_ptr db, GainBucket& gb, double base_overflow);

//최종 function
void bin_FM(dataBase_ptr db, int bin_num_x, int bin_num_y, double base_overflow);

}
#endif