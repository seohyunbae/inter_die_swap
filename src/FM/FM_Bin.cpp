#include "FM_Bin.h"

using namespace std;

namespace FM_Die{

Bin::Bin(Bin* bin, double coord_x, double coord_y){
    bin_index.x = bin->bin_index.x;
    bin_index.y = bin->bin_index.y;
    total_index = bin->total_index;
    lowerleft.x = coord_x;
    lowerleft.y = coord_y;
    bin_size.x = bin->bin_size.x;
    bin_size.y = bin->bin_size.y;
    bin_area = bin->bin_area;
}

//Bin::~Bin(){
//    inst_list[0].clear();
//    inst_list[1].clear();
//}

AllBin::~AllBin(){
    for(int i=0; i<bin_vec.size(); ++i){
        delete bin_vec[i];
        bin_vec.clear();
    }

    map<int, list<Bin*>*>::iterator itr;
    for(itr=bin_bucket.begin(); itr!=bin_bucket.end(); ++itr){
        itr->second->clear();
        delete itr->second;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//make and update bin
void init_bin(int x_num, int y_num, AllBin& ab, dataBase_ptr db){
    //ab update
    //bin size에 따른 제한을 둬야하나?
    ab.num_of_bin.x = x_num;
    ab.num_of_bin.y = y_num;
    ab.bin_num = x_num * y_num;
    ab.die_size.x = (db->dieDB->upperRightX - db->dieDB->lowerLeftX);
    ab.die_size.y = (db->dieDB->upperRightY - db->dieDB->lowerLeftY);
    ab.bin_size.x = ab.die_size.x / (double)ab.num_of_bin.x;
    ab.bin_size.y = ab.die_size.y / (double)ab.num_of_bin.y;
    ab.target_util[0] = db->dieDB->top_die->targetUtil;
    ab.target_util[1] = db->dieDB->bot_die->targetUtil;

    //make bin and bin vector
    double x_start = db->dieDB->lowerLeftX;
    double y_start = db->dieDB->lowerLeftY;
    for(int y=0; y<ab.num_of_bin.y; ++y){
        for(int x=0; x<ab.num_of_bin.x; ++x){
            //top
            Bin* bin = new Bin();
            bin->bin_index.x = x;
            bin->bin_index.y = y;
            bin->total_index = x + (y * ab.num_of_bin.x);
            bin->lowerleft.x = x_start + (double)(x * ab.bin_size.x);
            bin->lowerleft.y = y_start + (double)(y * ab.bin_size.y);
            bin->bin_size.x = ab.bin_size.x;
            bin->bin_size.y = ab.bin_size.y;   
            bin->bin_area = bin->bin_size.x * bin->bin_size.y; 
            ab.bin_vec.push_back(bin);
        }
    }

    //debug
    if(ab.bin_vec.size() != ab.bin_num){
        cout<<"error: bin vector's number of bin"<<endl;
        exit(0);
    }
}

void get_inst_idx(instance_ptr inst, AllBin& ab, dataBase_ptr db){
    int x = (int)floor(inst->center.x / ab.bin_size.x);
    int y = (int)floor(inst->center.y / ab.bin_size.y);
    inst->bin_index = x + (y * ab.num_of_bin.x);
    
    //debug
    double llx, lly, urx, ury;
    llx = ab.bin_vec[inst->bin_index]->lowerleft.x;
    lly = ab.bin_vec[inst->bin_index]->lowerleft.y;
    urx = llx + ab.bin_size.x;
    ury = lly + ab.bin_size.y;

    if(inst->center.x>urx || inst->center.y>ury || inst->center.x<llx || inst->center.y<lly){
        cout<<"error: instance center is not in bin[index]"<<endl;
        exit(0);
    }
    else if(inst->bin_index >= ab.bin_num){
        cout<<"error: instance's bin index overflow"<<endl;
        exit(0);
    }
}

void count_inst(instance_ptr inst, AllBin& ab, dataBase_ptr db){
    Bin* bin = ab.bin_vec[inst->bin_index];
    bin->inst_list[inst->dieNum].push_front(inst);
    inst->bin_inst_itr = bin->inst_list[inst->dieNum].begin();
    bin->sum_inst_area[inst->dieNum] += (unsigned int)inst->area;
    ++bin->inst_num[inst->dieNum];

    //debug
    if(bin->inst_list[inst->dieNum].size()!=bin->inst_num[inst->dieNum]){
        cout<<"error: top inst_num and list size not match"<<endl;
        exit(0);
    }
}

void cal_overflow(AllBin& ab, dataBase_ptr db){
    int total_inst[2] = {0, 0};
    unsigned int total_area[2] = {0, 0};
    //instance update for bin
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        get_inst_idx(inst, ab, db);
        count_inst(inst, ab, db);
    }
    //bin update and calculate overflow
    for(int i=0; i<ab.bin_vec.size(); ++i){
        Bin* bin = ab.bin_vec[i];
        bin->cur_util[0] = (double)(bin->sum_inst_area[0] / bin->bin_area);
        bin->cur_util[1] = (double)(bin->sum_inst_area[1] / bin->bin_area);
        double overflow_top = bin->cur_util[0]*(double)100 - ab.target_util[0];
        double overflow_bot = bin->cur_util[1]*(double)100 - ab.target_util[1];
        
        if(overflow_top > 0) bin->overflow[0] = overflow_top;
        else bin->overflow[0] = 0;

        if(overflow_bot > 0) bin->overflow[1] = overflow_bot;
        else bin->overflow[1] = 0;

        ab.overflow_sum[0] += bin->overflow[0];
        ab.overflow_sum[1] += bin->overflow[1];
        //debug
        total_inst[0] += bin->inst_num[0];
        total_inst[1] += bin->inst_num[1];
        total_area[0] += bin->sum_inst_area[0];
        total_area[1] += bin->sum_inst_area[1];
    }
    
    //debug and calculate overflow average
    if((total_inst[0]+total_inst[1]) != db->instanceDB->numInsts){
        cout<<"error: bin's total_inst != numInsts"<<endl;
        exit(0);
    }
    else if(total_inst[0] != db->dieDB->top_die->numInsts){
        cout<<"error: top bin's instance number"<<endl;
        exit(0);
    }
    else if(total_inst[1] != db->dieDB->bot_die->numInsts){
        cout<<"error: bot bin's instance number"<<endl;
        exit(0);
    }
    else if(total_area[0] != db->dieDB->top_die->curArea){
        cout<<"error: top bin's instance area"<<endl;
        exit(0);
    }
    else if(total_area[1] != db->dieDB->bot_die->curArea){
        cout<<"error: bot bin's instance area"<<endl;
        exit(0);
    }
    else{
        ab.overflow_avg[0] = ab.overflow_sum[0]/(double)ab.bin_num;
        ab.overflow_avg[1] = ab.overflow_sum[1]/(double)ab.bin_num;
    }
}

void before_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb){
    Bin* bin = ab.bin_vec[inst->bin_index];

    --bin->inst_num[inst->dieNum];
    bin->sum_inst_area[inst->dieNum] -= (unsigned int)inst->area;
    bin->inst_list[inst->dieNum].erase(inst->bin_inst_itr);
    ab.overflow_sum[inst->dieNum] -= bin->overflow[inst->dieNum];

    bin->cur_util[inst->dieNum] = (double)(bin->sum_inst_area[inst->dieNum]/bin->bin_area);
    double overflow = bin->cur_util[inst->dieNum]*(double)100 - ab.target_util[inst->dieNum];
    if(overflow > 0) bin->overflow[inst->dieNum] = overflow;
    else bin->overflow[inst->dieNum] = 0;
    ab.overflow_sum[inst->dieNum] += bin->overflow[inst->dieNum];
    ab.overflow_avg[inst->dieNum] = ab.overflow_sum[inst->dieNum] / (double)ab.bin_num;
}

void after_update_bin(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb){
    Bin* bin = ab.bin_vec[inst->bin_index];

    ++bin->inst_num[inst->dieNum];
    bin->sum_inst_area[inst->dieNum] += (unsigned int)inst->area;
    bin->inst_list[inst->dieNum].push_front(inst);
    inst->bin_inst_itr = bin->inst_list[inst->dieNum].begin();
    ab.overflow_sum[inst->dieNum] -= bin->overflow[inst->dieNum];

    bin->cur_util[inst->dieNum] = (double)(bin->sum_inst_area[inst->dieNum]/bin->bin_area);
    double overflow = bin->cur_util[inst->dieNum]*(double)100 - ab.target_util[inst->dieNum];
    if(overflow > 0) bin->overflow[inst->dieNum] = overflow;
    else bin->overflow[inst->dieNum] = 0;
    ab.overflow_sum[inst->dieNum] += bin->overflow[inst->dieNum];
    ab.overflow_avg[inst->dieNum] = ab.overflow_sum[inst->dieNum] / (double)ab.bin_num;
}

void make_bin(int x_num, int y_num, AllBin& ab, dataBase_ptr db){
    init_bin(x_num, y_num, ab, db);
    cal_overflow(ab, db);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//make and update bin bucket
/*
void before_update_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db){
    Bin* bin = ab.bin_vec[inst->bin_index];

    if(ab.bin_bucket.end()==ab.bin_bucket.find(bin->bin_bucket_key)){
        cout<<"error: bin bucket has no key"<<endl;
        exit(0);
    }
    ab.bin_bucket[bin->bin_bucket_key]->erase(inst->bin_bucket_itr);
}
*/

void update_bin_bucket(instance_ptr inst, AllBin& ab, dataBase_ptr db){
    Bin* bin = ab.bin_vec[inst->bin_index];
    //erase
    if(ab.bin_bucket.end()==ab.bin_bucket.find(bin->bin_bucket_key)){
        cout<<"error: bin bucket has no key"<<endl;
        exit(0);
    }
    ab.bin_bucket[bin->bin_bucket_key]->erase(inst->bin_bucket_itr);
    
    //push
    double diff = bin->overflow[0] - bin->overflow[1];
    if(diff > 0){
        bin->large_die = 0;
    }
    else{
        bin->large_die = 1;
        diff = -diff;
    }
    bin->bin_bucket_key = (int)floor(diff);

    if(ab.bin_bucket.end()==ab.bin_bucket.find(bin->bin_bucket_key)){
        ab.bin_bucket[bin->bin_bucket_key] = new list<Bin*>;
    }
    ab.bin_bucket[bin->bin_bucket_key]->push_front(bin);
    list<instance_ptr>::iterator itr;
    for(itr=bin->inst_list[bin->large_die].begin(); itr!=bin->inst_list[bin->large_die].end(); ++itr){
        (*itr)->bin_bucket_itr = ab.bin_bucket[bin->bin_bucket_key]->begin();
    }
    for(itr=bin->inst_list[!bin->large_die].begin(); itr!=bin->inst_list[!bin->large_die].end(); ++itr){
        (*itr)->bin_bucket_itr = ab.bin_bucket[bin->bin_bucket_key]->begin();
    }
    //반대편 die도 update해야하나?
}

void make_bin_bucket(AllBin& ab, dataBase_ptr db){
    //reset
    map<int, list<Bin*>*>::iterator itr;
    if(!ab.bin_bucket.empty()){
        for(itr=ab.bin_bucket.begin(); itr!=ab.bin_bucket.end(); ++itr){
            itr->second->clear();
        }
    }
    //make
    double diff;
    for(int i=0; i<ab.bin_vec.size(); ++i){
        Bin* bin = ab.bin_vec[i];
        diff = bin->overflow[0] - bin->overflow[1];
        if(diff > 0){
            bin->large_die = 0;
        }
        else{
            bin->large_die = 1;
            diff = -diff;
        }
        bin->bin_bucket_key = (int)floor(diff);

        if(ab.bin_bucket.end()==ab.bin_bucket.find(bin->bin_bucket_key)){
            ab.bin_bucket[bin->bin_bucket_key] = new list<Bin*>;
        }
        ab.bin_bucket[bin->bin_bucket_key]->push_front(bin);
        list<instance_ptr>::iterator itr;
        for(itr=bin->inst_list[bin->large_die].begin(); itr!=bin->inst_list[bin->large_die].end(); ++itr){
            (*itr)->bin_bucket_itr = ab.bin_bucket[bin->bin_bucket_key]->begin();
        }
        for(itr=bin->inst_list[!bin->large_die].begin(); itr!=bin->inst_list[!bin->large_die].end(); ++itr){
            (*itr)->bin_bucket_itr = ab.bin_bucket[bin->bin_bucket_key]->begin();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//init partition
bool bin_init_condition(dataBase_ptr db, int die_num){
    //die_num = 1 die gp가 이루어진 die
    if(die_num == 0){//top
        if(db->dieDB->top_die->curArea < db->dieDB->top_die->targetArea) return true;
    }
    else if(die_num == 1){//bot
        if(db->dieDB->bot_die->curArea < db->dieDB->bot_die->targetArea) return true;
    }
    else{
        cout<<"error: 1 die gp result die_num (init condition)"<<endl;
        exit(0);
    }
    return false;
}

void bin_init_db_parameter(dataBase_ptr db, GainBucket& gb, int die_num){
    //update die
    die_ptr top = db->dieDB->top_die;
    die_ptr bot = db->dieDB->bot_die;
    top->targetArea = (double)top->dieArea * (double)top->targetUtil / (double)100;
    bot->targetArea = (double)bot->dieArea * (double)bot->targetUtil / (double)100;
    //update instance
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        inst->gain = -(inst->numNets);
        inst->update = false;
        inst->fixed = false;
        inst->bestDie = die_num;
    }
    //update net
    for(int i=0; i<db->netDB->numNets; ++i){
        net_ptr net = db->netDB->net_array[i];
        net->cut = false;
        net->update = false;
        net->instance_distribution[die_num] = net->numInst;
        net->instance_distribution[1-die_num] = 0;
    }
}

void bin_init_partition(dataBase_ptr db, GainBucket& gb, int die_num){
    map<int, list<instance_ptr>*>::iterator itr;
    list<instance_ptr>::iterator l_itr;
    int count = 0;
    while(!bin_init_condition(db, die_num)){
        ++count;
        bool for_select_base = true;
        itr = gb.TGB.end();
        --itr;
        while(for_select_base){
            if(itr==gb.TGB.begin()){
                if(itr->second->empty()){
                    for_select_base = false;
                    break;
                }
            }
            if(!itr->second->empty()){
                l_itr = itr->second->begin();
                while(!basecell(*l_itr, db)){
                    ++l_itr;
                    if(l_itr == itr->second->end()) break;
                }
                if(l_itr == itr->second->end()){
                    if(itr == gb.TGB.begin()){
                        for_select_base = false;
                        break;
                    }
                    else --itr;
                }
                else if(basecell(*l_itr, db)){
                    move_and_update(*l_itr, db, gb, true, true, false, true);
                    for_select_base = false;
                }
                else --itr;
            }
            else --itr;
        }
    }
    print_state(db, gb);
    for_best_reset(db, gb, true, false, true);
    if(!normal_path(db, gb)){
        cout<<"error: check init partition"<<endl;
        exit(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//overflow partition
/*
void ovf_inst_net_update(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb){
    if(inst->fixed) return;
    inst->fixed = true;
    int cur_dieNum = inst->dieNum;
    int opp_dieNum = 1;
    if(cur_dieNum == 1) opp_dieNum = 0;

    inst2net_ptr i_net = inst->net_head;
    while(i_net){
        net_ptr net = (net_ptr)i_net->net;
        if(!net->update){
            net->update = true;
            net->instance_distribution[cur_dieNum] -= 1;
            net->instance_distribution[opp_dieNum] += 1;
            if(net->instance_distribution[0]<0 || net->instance_distribution[1]<0){
                cout<<"error: negative instance distribution"<<endl;
                exit(0);
            }
            if(net->cut){
                if(net->instance_distribution[0]==0 || )
            }
        }

        i_net = i_net->next;
    }
}
*/

double oppArea(instance_ptr inst, dataBase_ptr db, AllBin& ab){
    if(inst == nullptr){
        cout<<"error: oppArea, nullptr"<<endl;
        exit(0);
    }
    
    int opp_area;
    die_ptr opp_die;
    Bin* bin = ab.bin_vec[inst->bin_index];
    int to_dieNum;
    if(inst->dieNum == 1){//from bot to top
        opp_die = db->dieDB->top_die;
        to_dieNum = 0;
    }
    else if(inst->dieNum == 0){//from top to bot
        opp_die = db->dieDB->bot_die;
        to_dieNum = 1;
    }

    cellSpec_ptr tech = inst->masterCell->cellSpec_head;
    while(tech){
        if(!strcmp(tech->techName, opp_die->techName)){
            opp_area = (tech->sizeX)*(tech->sizeY);
            break;
        }
        tech = tech->next;
    }

    return (double)opp_area;
}

void ovf_mv_and_up(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb){
    if(inst==nullptr){
        cout<<"error: instance pointer is nullptr"<<endl;
        exit(0);
    }
    if(inst->fixed) return;

    before_update_bin(inst, ab, db, gb);
    move_and_update(inst, db, gb, true, true, false, true);
    after_update_bin(inst, ab, db, gb);
    update_bin_bucket(inst, ab, db);
}

bool overflow_condition(AllBin& ab, double base_overflow){
    bool top = ab.overflow_avg[0] <= base_overflow;
    bool bot = ab.overflow_avg[1] <= base_overflow;
    return top&&bot;
}
/*
instance_ptr find_basecell(AllBin& ab, Bin* bin, double area_diff, dataBase_ptr db){

}
*/
instance_ptr overflow_basecell(AllBin& ab, dataBase_ptr db, Bin* bin){
    double area_diff;
    if(bin->large_die == 0){//top->bot
        area_diff = db->dieDB->bot_die->targetArea - db->dieDB->bot_die->curArea;
    }
    else if(bin->large_die == 1){//bot->top
        area_diff = db->dieDB->top_die->targetArea - db->dieDB->top_die->curArea;
    }

    list<instance_ptr>::iterator itr;
    if(bin->inst_list[bin->large_die].empty()) return nullptr;
    itr = bin->inst_list[bin->large_die].begin();
    while((oppArea(*itr, db, ab) >= area_diff) || ((*itr)->fixed)){
        ++itr;
        if(itr == bin->inst_list[bin->large_die].end()) break;
    }

    if(itr == bin->inst_list[bin->large_die].end()){
        return nullptr;
    }
    else if((oppArea(*itr, db, ab) < area_diff) && !((*itr)->fixed)){
        return *itr;
    }
    else return nullptr;
}

void overflow_fm(AllBin& ab, dataBase_ptr db, GainBucket& gb, double base_overflow){
    map<int, list<Bin*>*>::iterator itr;
    list<Bin*>::iterator b_itr;
    instance_ptr inst;
    int count = 0;
    while(!overflow_condition(ab, base_overflow)){
        ++count;
        bool for_select_base = true;
        itr = ab.bin_bucket.end();
        --itr;
        while(for_select_base){
            if(itr == ab.bin_bucket.begin()){
                if(itr->second->empty()){
                    for_select_base = false;
                    break;
                }
            }
            if(!itr->second->empty()){
                b_itr = itr->second->begin();
                bool basecell = false;
                inst = overflow_basecell(ab, db, *b_itr);
                if(inst) basecell = true;
                while(!basecell){
                    ++b_itr;
                    if(b_itr == itr->second->end()) break;
                    else{
                        inst = overflow_basecell(ab, db, *b_itr);
                        if(inst) basecell = true;
                    }
                }
                if(b_itr == itr->second->end()){
                    if(itr == ab.bin_bucket.begin()){
                        for_select_base = false;
                        break;
                    }
                    else --itr;
                }
                else if(basecell){
                    ovf_mv_and_up(inst, ab, db, gb);
                    if(inst == nullptr) cout<<"inst = nullptr"<<endl;
                    cout<<(*b_itr)->bin_bucket_key<<"-"<<inst->instanceName<<": "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
                    for_select_base = false;
                }
                else --itr;
            }
            else --itr;
        }
        if(itr == ab.bin_bucket.begin()){
            if(itr->second->empty()){
                cout<<"error: larger base overflow needed"<<endl;
                exit(0);
            }
            else if(itr->second->end() == b_itr){
                if(inst == nullptr){
                    cout<<"error: larger base overflow needed"<<endl;
                    exit(0);
                }
            }
        }
    }
    bin_for_best_reset(db, gb, ab);

    if(db->dieDB->top_die->curArea > db->dieDB->top_die->targetArea){
        cout<<"error: top die overflow"<<endl;
        exit(0);
    }
    else if(db->dieDB->bot_die->curArea > db->dieDB->bot_die->targetArea){
        cout<<"error: bot die overflow"<<endl;
        exit(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//bi partition
void bin_mv_and_up(instance_ptr inst, AllBin& ab, dataBase_ptr db, GainBucket& gb){
    if(!inst){
        cout<<"error: instance pointer is nullptr"<<endl;
        exit(0);
    }
    if(inst->fixed) return;

    before_update_bin(inst, ab, db, gb);
    move_and_update(inst, db, gb, false, true, false, true);
    after_update_bin(inst, ab, db, gb);
}

bool bin_basecell(instance_ptr inst, dataBase_ptr db, AllBin& ab, double base_overflow){
    bool die_util = false;
    bool bin_util = false;

    int oppArea;
    die_ptr opp_die;
    Bin* bin = ab.bin_vec[inst->bin_index];
    int to_dieNum;
    if(inst->dieNum == 1){//from bot to top
        opp_die = db->dieDB->top_die;
        to_dieNum = 0;
    }
    else if(inst->dieNum == 0){//from top to bot
        opp_die = db->dieDB->bot_die;
        to_dieNum = 1;
    }

    cellSpec_ptr tech = inst->masterCell->cellSpec_head;
    while(tech){
        if(!strcmp(tech->techName, opp_die->techName)){
            oppArea = (tech->sizeX)*(tech->sizeY);
            break;
        }
        tech = tech->next;
    }

    if((opp_die->curArea + oppArea) < opp_die->targetArea) die_util = true;

    double expt_ovf_avg = (ab.overflow_sum[to_dieNum]+((double)oppArea*(double)100/bin->bin_area))/(double)ab.bin_num;
    if(expt_ovf_avg < base_overflow) bin_util = true;

    return die_util&&bin_util;
}

void bin_bi_partition(AllBin& ab, dataBase_ptr db, GainBucket& gb, double base_overflow){
    int count = 0;
    int best = db->netDB->numNets+1;
    map<int, list<instance_ptr>*>::iterator itr;
    list<instance_ptr>::iterator l_itr;
    while(best > gb.min_cut_num){
        ++count;
        best = gb.min_cut_num;
        bool path = true;
        while(path){
            bool for_select_base = true;
            itr = gb.TGB.end();
            --itr;
            while(for_select_base){
                if(itr == gb.TGB.begin()){
                    if(itr->second->empty()){
                        for_select_base = false;
                        path = false;
                        break;
                    }
                }
                if(!itr->second->empty()){
                    l_itr = itr->second->begin();
                    while(!bin_basecell(*l_itr, db, ab, base_overflow)){
                        ++l_itr;
                        if(l_itr == itr->second->end()) break;
                    }
                    if(l_itr == itr->second->end()){
                        if(itr == gb.TGB.begin()){
                            for_select_base = false;
                            path = false;
                            break;
                        }
                        else --itr;
                    }
                    else if(bin_basecell(*l_itr, db, ab, base_overflow)){
                        bin_mv_and_up(*l_itr, ab, db, gb);
                        for_select_base = false;
                    }
                    else --itr;
                }
                else --itr;
            }
        }
        bin_for_best_reset(db, gb, ab);
        //for debugging
        if(true){
            if(!normal_path(db, gb)){
                cout<<"error: bi-partition. check the code"<<endl;
                break;
                exit(0);
            }
        }
        cout<<"path-count: "<<count<<", overflow: "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//bin based partition
void bin_for_best_die_move(instance_ptr inst, dataBase_ptr db, GainBucket& gb, AllBin& ab){
    bool move = inst->bestDie != inst->dieNum;
    if(move) before_update_bin(inst, ab, db, gb);
    for_best_die_move(inst, db, gb);
    if(move) after_update_bin(inst, ab, db, gb);
}

bool bin_for_best_reset(dataBase_ptr db, GainBucket& gb, AllBin& ab){
    //best 상태로 옮겨주기
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        bin_for_best_die_move(db->instanceDB->inst_array[i], db, gb, ab);
    }
    //best 상태에서 gain 계산
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        for_best_make_gain(db->instanceDB->inst_array[i], db);
    }
    //계산된 gain으로 다음 iteration위해 gainbucket 만들어주기
    init_gainbucket(db, gb, true, false, true);
    //for debugging
    if(gb.cut_net_num == gb.min_cut_num ) return true;
    else{
        cout<<"error: reset for best is failed"<<endl;
        exit(0);
        return false;
    }
}

void bin_FM(dataBase_ptr db, int bin_num_x, int bin_num_y, double base_overflow){
    cout<<"=================Bin-based Die_Partition: start=================="<<'\n'<<'\n'<<endl;
    
    int die_num = db->instanceDB->inst_array[0]->dieNum;
    cout<<die_num<<endl;
    cout<<"------------------Die Partition State-----------------------"<<endl;
    cout<<"instance: "<<db->instanceDB->numInsts<<", net: "<<db->netDB->numNets<<endl;
    //cout<<"min cut: "<<gb.min_cut_num<<", current cut: "<<gb.cut_net_num<<endl;
    cout<<"die area: "<<db->dieDB->top_die->dieArea<<endl;
    cout<<"inst num: ("<<db->dieDB->top_die->numInsts<<", "<<db->dieDB->bot_die->numInsts<<")"<<endl;
    cout<<"current area/target area: top-("<<db->dieDB->top_die->curArea<<"/"<<db->dieDB->top_die->targetArea;
    cout<<"), bot-("<<db->dieDB->bot_die->curArea<<"/"<<db->dieDB->bot_die->targetArea<<")"<<endl;
    cout<<"current util/target util: top-("<<db->dieDB->top_die->curUtil<<"/"<<(double)db->dieDB->top_die->targetUtil/(double)100;
    cout<<"), bot-("<<db->dieDB->bot_die->curUtil<<"/"<<(double)db->dieDB->bot_die->targetUtil/(double)100<<")"<<endl;
    cout<<"------------------------------------------------------------"<<endl;


    cout<<"Bin-based Die_Partition: make gain bucket start"<<endl;
    GainBucket gb;
    bin_init_db_parameter(db, gb, die_num);
    init_gainbucket(db, gb, true, false, true);
    cout<<"Bin-based Die_Partition: make gain bucket complete"<<'\n'<<'\n'<<endl;

    //init partition, 이전 fm과 동일
    cout<<"Bin-based Die_Partition: init partition start"<<endl;
    bin_init_partition(db, gb, die_num);
    print_state(db, gb);
    cout<<"Bin-based Die_Partition: init partition complete"<<'\n'<<'\n'<<endl;
    
    //bin init
    cout<<"Bin-based Die_Partition: make bin start"<<endl;
    AllBin ab;
    make_bin(bin_num_x, bin_num_y, ab, db);
    cout<<"overflow: "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
    cout<<"Bin-based Die_Partition: make bin complete"<<'\n'<<'\n'<<endl;

    //bin based overflow fm
    //make bin bucket
    cout<<"Bin-based Die_Partition: make bin bucket start"<<endl;
    make_bin_bucket(ab, db);
    cout<<"overflow: "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
    cout<<"Bin-based Die_Partition: make bin bucket complete"<<'\n'<<'\n'<<endl;
    //bin based overflow fm
    cout<<"Bin-based Die_Partition: overflow fm start"<<endl;
    overflow_fm(ab, db, gb, base_overflow);
    print_state(db, gb);
    cout<<"overflow: "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
    cout<<"Bin-based Die_Partition: overflow fm complete"<<'\n'<<'\n'<<endl;

    //bin based bi partition
    cout<<"Bin-based Die_Partition: bi-partition start"<<endl;
    //make_bin_bucket(allbin, db);
    bin_bi_partition(ab, db, gb, base_overflow);
    print_state(db, gb);
    cout<<"overflow: "<<ab.overflow_avg[0]<<"/"<<ab.overflow_avg[1]<<endl;
    
    if(end_path(db, gb)){
        //print_state(db, gb);
        //end_partition(db);
        cout<<"Bin-based Die_Partition: bi-partition complete"<<'\n'<<'\n'<<endl;
    }
    else{
        cout<<"Die_partition: error. check the code"<<'\n'<<'\n'<<endl;
        exit(0);
    }
 
    cout<<"================Bin-based Die_Partition: complete================"<<endl;
}

}