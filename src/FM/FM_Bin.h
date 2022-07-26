#ifndef _FM_BIN_
#define _FM_BIN_

#include "FM_Die.h"

namespace FM_Die{

//bin 1개에 top, bot 정보 다 들고 있기.
class Bin{
    public:
        struct POS bin_index; //index의 x, y좌표
        int total_index; //vector 내에서의 좌표
        struct FPOS lowerleft;
        struct FSIZE bin_size;
        double bin_area;

        int inst_num[2];
        unsigned int sum_inst_area[2];
        double cur_util[2];
        double overflow[2]; // %단위
        std::list<instance_ptr> inst_list[2];

        int bin_bucket_key;
        int large_die; //top_overflow > bot_overflow 면 0(top), else: 1(bot)

        Bin(){
            inst_num[0] = 0;
            inst_num[1] = 0;
            sum_inst_area[0] = 0;
            sum_inst_area[1] = 0;
            bin_bucket_key = 0;
        };//top bin
        Bin(Bin* bin, double coord_x, double coord_y); //bot bin
        //~Bin();
};

class AllBin{
    public:
        std::vector<Bin*> bin_vec;

        std::map<int, std::list<Bin*>*> bin_bucket;

        int bin_num;
        struct POS num_of_bin;
        struct FSIZE die_size;
        struct FSIZE bin_size;
        double target_util[2]; //0:top, 1:bot

        double overflow_sum[2];
        double overflow_avg[2];

        AllBin(){
            overflow_sum[0] = 0;
            overflow_sum[1] = 0;
        };
        ~AllBin();
};

//make and update bin
void init_bin(int x_num, int y_num, AllBin& ab, dataBase_ptr db);
void get_inst_idx(instance_ptr inst, AllBin& ab, dataBase_ptr db);
//void get_bin_idx(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void count_inst(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void cal_overflow(AllBin& ab, dataBase_ptr db);
void before_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
void after_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
void make_bin(int x_num, int y_num, AllBin& ab, dataBase_ptr db);

//make and update bin bucket
//void before_update_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void update_bin_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void make_bin_bucket(AllBin& ab, dataBase_ptr db);

//init partition
bool bin_init_condition(dataBase_ptr db, int die_num);
void bin_init_db_parameter(dataBase_ptr db, GainBucket& gb, int die_num);
void bin_init_partition(dataBase_ptr db, GainBucket& gb, int die_num);

//overflow partition
//void ovf_inst_net_update(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
double oppArea(instance_ptr inst, dataBase_ptr db, AllBin& ab);
void ovf_mv_and_up(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
bool overflow_condition(AllBin& ab, double base_overflow);
//instance_ptr find_basecell(AllBin& ab, Bin* bin, double area_diff, dataBase_ptr db);
instance_ptr overflow_basecell(AllBin& ab, dataBase_ptr db, Bin* bin);
void overflow_fm(AllBin& ab, dataBase_ptr db, GainBucket& gb, double base_overflow);

//bi partition
void bin_mv_and_up(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
bool bin_basecell(instance_ptr inst, dataBase_ptr db, AllBin& ab, double base_overflow);
void bin_bi_partition(AllBin& ab, dataBase_ptr db, GainBucket& gb, double base_overflow);

//bin based partition
void bin_for_best_die_move(instance_ptr inst, dataBase_ptr db, GainBucket& gb, AllBin& ab);
bool bin_for_best_reset(dataBase_ptr db, GainBucket& gb, AllBin& ab);
void bin_FM(dataBase_ptr db, int bin_num_x, int bin_num_y, double base_overflow);

//debug
void check_overflow_avg();

}
#endif