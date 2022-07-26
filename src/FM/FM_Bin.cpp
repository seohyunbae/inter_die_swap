#include "FM_Bin.h"

using namespace std;

namespace FM_Die{

Bin::Bin(Bin* bin){
    
}

//make bin
void init_bin(int x_num, int y_num, AllBin& ab, dataBase_ptr db);
void get_inst_idx(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void get_bin_idx(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void count_inst(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void cal_overflow(AllBin& ab, dataBase_ptr db);
void before_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
void after_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);

//make bin bucket
void before_update_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void after_update_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db);
void make_bin_bucket(AllBin& ab, dataBase_ptr db);

//init partition
bool bin_init_condition(dataBase_ptr db, int die_num);
void bin_init_db_parameter(dataBase_ptr db, GainBucket& gb, int die_num);
void bin_init_partition(dataBase_ptr db, GainBucket& gb, int die_num);

//overflow partition
void ovf_mv_and_up(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb);
bool overflow_condition(AllBin& ab, double base_overflow);
instance_ptr find_basecell(AllBin& ab, Bin* bin, double area_diff, dataBase_ptr db);
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







}