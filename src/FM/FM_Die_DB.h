#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include <list>
#include <string>
#include "../DB/DataBase.h"
#include "../DB/global.h"
#ifndef _FM_DB_
#define _FM_DB_

namespace FM_Die_Partition{

class FM;
class FM_Net;
class FM_Instance;
class FM_GainBucket;
class FM_Die;

class FM
{
    public:
        int inst_num;
        int net_num;
        int die_num;
        int tech_num;

        std::unordered_map<std::string, FM_Instance*> Inst_hash;
        std::unordered_map<std::string, FM_Net*> Net_hash;
        std::vector<FM_Instance*> Inst_vec;
        std::vector<FM_Net*> Net_vec;
        std::vector<FM_Die*> Die_vec; //Die_vec[0] = top, Die_vec[1] = bottom
        FM_GainBucket* GB;

        FM(dataBase_ptr db); //top에 몰아 넣는 init까지 생성자에 넣어봄
        ~FM();

        //void init_Inst_Net_vec_and_gain(dataBase_ptr db);
        void init_gainbucket();
        //1번 돌아가고 나서 다음 반복을 위해 best Die로 옮겨주는 함수.
        bool for_best_reset();
};

class FM_Instance
{
    public:
        std::string instName;
        
        std::unordered_map<std::string, int> area; //string = techName
        int gain;
        bool fixed;

        FM_Die* cur_Die;
        FM_Die* opp_Die;
        FM_Die* best_Die;

        std::list<FM_Instance*>::iterator GB_itr;
        std::vector<FM_Net*> Net_vec;

        friend std::ostream& operator<<(std::ostream& os, FM_Instance* t);

        FM_Instance(instance_ptr inst, dataBase_ptr db, FM* fm);
        //처음 생성자에서 만들 때 부터 싹 다 top die에 몰아 넣기
        void move_and_update(FM_GainBucket* gb, bool init, FM& fm);
        //best 상태로 만들기 위한 함수
        void for_best_Die_move(FM_GainBucket* gb);
        void for_best_make_gain();
};

class FM_Net
{
    public:
        std::string netName;
        bool cut; // True: cut, False: not cut

        std::unordered_map<std::string, int> inst_dist; //string = dieName (cur_Die* 통해서 접근)
        std::vector<FM_Instance*> Inst_vec;

        friend std::ostream& operator<<(std::ostream& os, FM_Net* t);

        FM_Net(net_ptr net, FM* fm, int inst_num);
};

class FM_GainBucket
{
    public:
        std::map<int, std::list<FM_Instance*>*> TGB; //total gain bucket
        int cut_net_num; //current
        int min_cut_num; //best

        friend std::ostream& operator<<(std::ostream& os, FM_GainBucket* t);

        bool min_cut_update();
        FM_GainBucket(): cut_net_num(0), min_cut_num(0) {};
        ~FM_GainBucket();
};

class FM_Die
{
    public:
        std::string dieName;
        std::string techName;
        int instNum;
        unsigned int size; //die size
        unsigned int targetArea; // targetArea = targetUtil*size
        unsigned int area; // util = area/size

        friend std::ostream& operator<<(std::ostream& os, const FM_Die* t);

        FM_Die(die_ptr die);
};
}

#endif