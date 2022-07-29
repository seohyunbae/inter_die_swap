#ifndef _FM_DIE_
#define _FM_DIE_

#include "../DB/global.h"
#include "../DB/DataBase.h"

//for random init
#include <random>

namespace FM_Die{

///////////////////////////Data Structure///////////////////////
class GainBucket{
    public:
        //total gain bucket
        std::map<int, std::list<instance_ptr>*> TGB;
        int cut_net_num;
        int min_cut_num;

        bool min_cut_update();

        friend std::ostream& operator<<(std::ostream& os, GainBucket& t);

        GainBucket(): cut_net_num(0), min_cut_num(0) {};
        ~GainBucket(); //for free TGB's second(= list pointer)
};

///////////////////////////FM DB Function/////////////////////////
//FM에 필요한 data들을 DB에 update해줌.
void init_db_parameter(dataBase_ptr db, GainBucket& gb);
//in DB
//gain bucket을 만들어주는 함수
void init_gainbucket(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug);
//after iteration, make best state for next iteration
bool for_best_reset(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug);

//in instance
//move and update selected instance
void move_and_update(instance_ptr inst, dataBase_ptr db, GainBucket& gb, int init, bool fifo_init, bool fifo_update, bool debug);
//move instance for making best state for next iteration
void for_best_die_move(instance_ptr inst, dataBase_ptr db, GainBucket& gb);
//best_die_move 이후에 gain을 계산 해주는 함수
void for_best_make_gain(instance_ptr inst, dataBase_ptr db);

///////////////////////////FM partition function///////////////////
//for partition
//utilization constraint를 만족하는지 확인해주는 함수
bool basecell(instance_ptr inst, dataBase_ptr db);
//init partition에서 top die의 utilization 조건이 만족되었는지 확인
bool init_condition(dataBase_ptr db);
//모든 instance가 top에 있다고 가정하고 init partition
void init_partition(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug);
//for test
void init_random(dataBase_ptr db, GainBucket& gb);
//bi_partition
void bi_partition(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug);
//전체 함수
//option
//0. random: for test
//1. fifo_init: gabinbucket init 시에 FIFO 쓸 것인지 LIFO쓸 것인지
//2. fifo_update: gain update시에 FIFO쓸 것인지 LIFO쓸 것인지.
//3. debug: debug function들의 작동. true일 경우 디버깅 함수 작동
// best choice: random-false, fifo_init-true, fifo_update-false, debug-false
void die_partition(dataBase_ptr db, bool random, bool fifo_init, bool fifo_update, bool debug);

//new
//for memory / dummy
void end_partition(dataBase_ptr db);

//for debugging
//간단하게 state를 나타내주는 함수
void print_state(dataBase_ptr db, GainBucket& gb);
//각 die에 속해 있는 instance의 area합이 맞게 계산 되었는지 확안.
bool size_check(dataBase_ptr db, GainBucket& gb);
//min_cut 및 net별로 instance distribution이 제대로 계산 되었는지 확인
bool mincut_check(dataBase_ptr db, GainBucket& gb);
//정상적인 path가 진행되었는지 확인
bool normal_path(dataBase_ptr db, GainBucket& gb);
//util이 제대로 계산 되었는지 확인
bool util_check(dataBase_ptr db, GainBucket& gb);
//전체 path가 끝났을 때 정상적으로 끝났는지 확인.
bool end_path(dataBase_ptr db, GainBucket& gb);

}
#endif