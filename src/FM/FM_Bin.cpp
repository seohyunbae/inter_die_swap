#include "FM_Bin.h"

using namespace std;

namespace FM_Die{

Bin::Bin(Bin* bin){
    bin_idx = bin->bin_idx;
    size_x = bin->size_x;
    size_y = bin->size_y;
    coord_x = bin->coord_x;
    coord_y = bin->coord_y;
    bin_area = bin->bin_area;

    die_num = 1;
}

AllBin::AllBin(int x_num, int y_num, dataBase_ptr db){
    x_bin_num = x_num;
    y_bin_num = y_num;

    die_ptr top = db->dieDB->top_die;
    die_size_x = (double)top->upperRightX - (double)top->lowerLeftX;
    die_size_y = (double)top->upperRightY - (double)top->lowerLeftY;
    target_util[0] = (double)top->targetUtil;
    target_util[1] = (double)db->dieDB->bot_die->targetUtil;

    bin_size_x = die_size_x / (double)x_num;
    bin_size_y = die_size_y / (double)y_num;
    bin_num = x_num * y_num;

    //make bin
    int bin_coord_y=0;
    int bin_coord_x=0;
    int index = 0;
    for(int y=0; y<y_num; ++y){
        for(int x=0; x<x_num; ++x){
            Bin* bin = new Bin();
            bin->die_num = 0;
            top_bin_vec.push_back(bin);
            if(x<x_num-1){
                bin->coord_x = bin_coord_x;
                bin->coord_y = bin_coord_y;
                bin_coord_x += bin_size_x; //
                bin->size_x = bin_size_x;
                bin->size_y = bin_size_y;
                bin->bin_area = bin->size_x * bin->size_y;
            }
            else if(x==x_num-1){
                if(y<y_num-1){
                    bin->coord_x = bin_coord_x;
                    bin->coord_y = bin_coord_y;
                    bin->size_x = die_size_x - bin->coord_x;
                    bin->size_y = bin_size_y;
                    bin->bin_area = bin->size_x * bin->size_y;
                }
                else if(y==y_num-1){
                    bin->coord_x = bin_coord_x;
                    bin->coord_y = bin_coord_y;
                    bin->size_x = die_size_x - bin->coord_x;
                    bin->size_y = die_size_y - bin->coord_y;
                    bin->bin_area = bin->size_x * bin->size_y;
                }
            }
            bin->bin_idx = index;
            ++index;
            bot_bin_vec.push_back(new Bin(bin));
        }
        bin_coord_x = 0;
        bin_coord_y += bin_size_y;
    }

    //debug
    if(top_bin_vec.size()!=bin_num){
        cout<<"error: in allbin constructor, top bin"<<endl;
        exit(0);
    }
    else if(bot_bin_vec.size()!=bin_num){
        cout<<"error: in allbin constructor, bot bin"<<endl;
        exit(0);
    }
}

AllBin::~AllBin(){
    vector<Bin*>::iterator itr;
    for(itr=top_bin_vec.begin(); itr!=top_bin_vec.end(); ++itr){
        delete *itr;
    }
    for(itr=bot_bin_vec.begin(); itr!=bot_bin_vec.end(); ++itr){
        delete *itr;
    }
    map<int, list<Bin*>*>::iterator itr_;
    for(itr_=bin_bucket.begin(); itr_!=bin_bucket.end(); ++itr_){
        itr_->second->clear();
        delete itr_->second;
    }
}

bool first_count_inst(instance_ptr inst, AllBin& allbin, dataBase_ptr db){
    double inst_x = inst->center.x;
    double inst_y = inst->center.y;
    int x_idx = (int)floor(inst_x/allbin.bin_size_x);
    int y_idx = (int)floor(inst_y/allbin.bin_size_y);
    int index = x_idx + y_idx * allbin.x_bin_num;
    if(index >= allbin.bin_num){
        cout<<"error: first count for instance in bin, index overflow"<<endl;   
        exit(0);
    }
    inst->bin_index = index;

    if(inst->dieNum == 0){//top
        Bin* bin = allbin.top_bin_vec[index];
        if(bin->bin_idx != index){
            cout<<"error: top bin index count error"<<endl;
            exit(0);
        }
        bin->inst_list.push_front(inst);
        inst->bin_inst_itr = bin->inst_list.begin();
        bin->sum_inst_area += inst->area;
        ++bin->inst_num; 
        return true;
    }
    else if(inst->dieNum == 1){//bot
        Bin* bin = allbin.bot_bin_vec[index];
        if(bin->bin_idx != index){
            cout<<"error: bot bin index count error"<<endl;
            exit(0);
        }
        bin->inst_list.push_front(inst);
        inst->bin_inst_itr = bin->inst_list.begin();
        bin->sum_inst_area += inst->area;
        ++bin->inst_num;
        return false;
    }

    return true;
}

void first_cal_overflow(AllBin& allbin, dataBase_ptr db){
    int total_top_inst = 0;
    int total_bot_inst = 0;
    allbin.ovf_sum[0] = 0;
    allbin.ovf_sum[1] = 0;
    double total_top_area = 0;
    double total_bot_area = 0;

    for(int i=0; i<allbin.top_bin_vec.size(); ++i){
        Bin* bin = allbin.top_bin_vec[i];
        bin->cur_util = bin->sum_inst_area/bin->bin_area;
        total_top_inst += bin->inst_num;
        total_top_area += bin->sum_inst_area;
        
        if(bin->die_num == 1){
            cout<<"error: bot bin in top_bin_vector"<<endl;
            exit(0);
        }
        else if(bin->cur_util > allbin.target_util[bin->die_num]){
            bin->overflow = (bin->cur_util - allbin.target_util[bin->die_num])*100;
            allbin.ovf_sum[0] += bin->overflow;
        }
        else bin->overflow = 0;
    }
    for(int i=0; i<allbin.bot_bin_vec.size(); ++i){
        Bin* bin = allbin.bot_bin_vec[i];
        bin->cur_util = bin->sum_inst_area/bin->bin_area;
        total_bot_inst += bin->inst_num;
        total_bot_area += bin->sum_inst_area;
        
        if(bin->die_num == 0){
            cout<<"error: top bin in bot_bin_vector"<<endl;
            exit(0);
        }
        else if(bin->cur_util > allbin.target_util[bin->die_num]){
            bin->overflow = (bin->cur_util - allbin.target_util[bin->die_num])*100;
            allbin.ovf_sum[1] += bin->overflow;
        }
        else bin->overflow = 0;
    }

    //debug and calculate overflow average
    if((total_top_inst+total_bot_inst) != db->instanceDB->numInsts){
        cout<<"error: bin's total_inst != numInsts"<<endl;
        exit(0);
    }
    else if(total_top_inst != db->dieDB->top_die->numInsts){
        cout<<"error: top bin's instance number"<<endl;
        exit(0);
    }
    else if(total_bot_inst != db->dieDB->bot_die->numInsts){
        cout<<"error: bot bin's instance number"<<endl;
        exit(0);
    }
    else if(total_top_area != db->dieDB->top_die->curArea){
        cout<<"error: top bin's instance area"<<endl;
        exit(0);
    }
    else if(total_bot_area != db->dieDB->bot_die->curArea){
        cout<<"error: bot bin's instance area"<<endl;
        exit(0);
    }
    else{
        allbin.ovf_avg[0] = allbin.ovf_sum[0]/(double)allbin.bin_num;
        allbin.ovf_avg[1] = allbin.ovf_sum[1]/(double)allbin.bin_num;
    }
}

void init_bin(AllBin& allbin, dataBase_ptr db){
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        bool base = first_count_inst(inst, allbin, db);
    }
    first_cal_overflow(allbin, db);
}

void before_inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db){
    Bin* bin;
    //bin bucket update
    double ovf_dif;
    int ovf_real;
    ovf_dif = allbin.top_bin_vec[inst->bin_index]->overflow - allbin.bot_bin_vec[inst->bin_index]->overflow;
    if(ovf_dif >= 0){
        ovf_real = (int)floor(ovf_dif);
        bin = allbin.top_bin_vec[inst->bin_index];
    }
    else if(ovf_dif < 0){
        ovf_real = (int)floor(-ovf_dif);
        bin = allbin.bot_bin_vec[inst->bin_index];
    }

    if(allbin.bin_bucket.end()==allbin.bin_bucket.find(ovf_real)){
        cout<<"error: bin bucket"<<endl;
        exit(0);
    }
    allbin.bin_bucket[ovf_real]->erase(inst->bin_bucket_itr);

    //bin and instance update
    if(inst->dieNum == 0){//top
        bin = allbin.top_bin_vec[inst->bin_index];
    }
    else if(inst->dieNum == 1){//bot
        bin = allbin.bot_bin_vec[inst->bin_index];
    }

    --bin->inst_num;
    bin->sum_inst_area -= inst->area;
    bin->inst_list.erase(inst->bin_inst_itr);
    bin->cur_util = bin->sum_inst_area/bin->bin_area;
     
    allbin.ovf_sum[bin->die_num] -= bin->overflow;

    if(bin->cur_util > allbin.target_util[bin->die_num]){
        bin->overflow = (bin->cur_util - allbin.target_util[bin->die_num])*100;
        allbin.ovf_sum[bin->die_num] += bin->overflow;
    }
    else bin->overflow = 0;

    allbin.ovf_avg[bin->die_num] = allbin.ovf_sum[bin->die_num]/(double)allbin.bin_num;
}

void after_inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db){
    Bin* bin;
    //bin and instance update
    if(inst->dieNum == 0){//top
        bin = allbin.top_bin_vec[inst->bin_index];
    }
    else if(inst->dieNum == 1){//bot
        bin = allbin.bot_bin_vec[inst->bin_index];
    }

    ++bin->inst_num;
    bin->sum_inst_area += inst->area;
    bin->inst_list.push_front(inst);
    inst->bin_inst_itr = bin->inst_list.begin();
    bin->cur_util = bin->sum_inst_area/bin->bin_area;

    allbin.ovf_sum[bin->die_num] -= bin->overflow;

    if(bin->cur_util > allbin.target_util[bin->die_num]){
        bin->overflow = (bin->cur_util - allbin.target_util[bin->die_num])*100;
        allbin.ovf_sum[bin->die_num] += bin->overflow;
    }
    else bin->overflow = 0;

    allbin.ovf_avg[bin->die_num] = allbin.ovf_sum[bin->die_num]/(double)allbin.bin_num;

    //bin bucket update
    double ovf_dif;
    int ovf_real;
    ovf_dif = allbin.top_bin_vec[inst->bin_index]->overflow - allbin.bot_bin_vec[inst->bin_index]->overflow;
    if(ovf_dif >= 0){
        ovf_real = (int)floor(ovf_dif);
        bin = allbin.top_bin_vec[inst->bin_index];
    }
    else if(ovf_dif < 0){
        ovf_real = (int)floor(-ovf_dif);
        bin = allbin.bot_bin_vec[inst->bin_index];
    }

    if(allbin.bin_bucket.end()==allbin.bin_bucket.find(ovf_real)){
        allbin.bin_bucket[ovf_real] = new list<Bin*>;
    }
    allbin.bin_bucket[ovf_real]->push_front(bin);
    list<instance_ptr>::iterator i_itr;
    for(i_itr=bin->inst_list.begin(); i_itr!=bin->inst_list.end(); ++i_itr){
        (*i_itr)->bin_bucket_itr = allbin.bin_bucket[ovf_real]->begin();
    }
}

void ovf_mv_and_up(instance_ptr inst, AllBin& allbin, dataBase_ptr db, GainBucket& gb){
    double x = inst->center.x;
    double y = inst->center.y;

    before_inst_bin_update(inst, allbin, db);
    swap_instance_to_other_die(db, inst);
    after_inst_bin_update(inst, allbin, db);

    if(x != inst->center.x){
        cout<<"error: position changed"<<endl;
        exit(0);
    }
    else if(y != inst->center.y){
        cout<<"error: position changed"<<endl;
        exit(0);
    }
}

bool bin_init_condition(dataBase_ptr db, int die_num){
    if(die_num == 0){
        if(db->dieDB->top_die->curArea <= db->dieDB->top_die->targetArea) return true;
        return false;
    }
    else if(die_num == 1){
        if(db->dieDB->bot_die->curArea <= db->dieDB->bot_die->targetArea) return true;
        return false;
    }
    return false;
}

void bin_init_db_parameter(dataBase_ptr db, GainBucket& gb, int die_num){
    if(die_num == 0){
        //update die
        die_ptr top = db->dieDB->top_die;
        die_ptr bot = db->dieDB->bot_die;
        top->targetArea = (double)top->dieArea * (double)top->targetUtil / (double)100;
        bot->targetArea = (double)bot->dieArea * (double)bot->targetUtil / (double)100;

        for(int i=0; i<db->instanceDB->numInsts; ++i){
            //update instance
            instance_ptr inst = db->instanceDB->inst_array[i];
            inst->gain = -(inst->numNets); //instance 당 연결된 net의 수?
            inst->update = false;
            inst->fixed = false;
            inst->bestDie = 0; //top

            //make whole instance move to top die
            place_instance_in_die(db, 1, inst);
        }

        //update net
        for(int i=0; i<db->netDB->numNets; ++i){
            net_ptr net = db->netDB->net_array[i];
            net->cut = false;
            net->update = false;
            net->instance_distribution[0] = net->numInst; //top
            net->instance_distribution[1] = 0; //bot
        }
    }
    else if(die_num == 1){
        //update die
        die_ptr top = db->dieDB->top_die;
        die_ptr bot = db->dieDB->bot_die;
        top->targetArea = (double)top->dieArea * (double)top->targetUtil / (double)100;
        bot->targetArea = (double)bot->dieArea * (double)bot->targetUtil / (double)100;

        for(int i=0; i<db->instanceDB->numInsts; ++i){
            //update instance
            instance_ptr inst = db->instanceDB->inst_array[i];
            inst->gain = -(inst->numNets); //instance 당 연결된 net의 수?
            inst->update = false;
            inst->fixed = false;
            inst->bestDie = 0; //top

            //make whole instance move to top die
            place_instance_in_die(db, 0, inst);
        }

        //update net
        for(int i=0; i<db->netDB->numNets; ++i){
            net_ptr net = db->netDB->net_array[i];
            net->cut = false;
            net->update = false;
            net->instance_distribution[1] = net->numInst; //bot
            net->instance_distribution[0] = 0; //top
        }
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
            if(itr == gb.TGB.begin()){
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
                    move_and_update((*l_itr), db, gb, true, true, false, true);
                    for_select_base = false;
                }
                else --itr;
            }
            else --itr;
        }
    }
    for_best_reset(db, gb, true, false, true);
    //for debugging
    if(true){
        if(!normal_path(db, gb)){
            cout<<"error: check init part code"<<endl;
            exit(0);
        }
    }
}

void make_bin_bucket(AllBin& allbin, dataBase_ptr db){
    map<int, list<Bin*>*>::iterator itr;
    if(!allbin.bin_bucket.empty()){
        for(itr=allbin.bin_bucket.begin(); itr!=allbin.bin_bucket.end(); ++itr){
            itr->second->clear();
        }
    }
    double ovf_dif;
    int ovf_real;
    for(int i=0; i<allbin.top_bin_vec.size(); ++i){
        ovf_dif = allbin.top_bin_vec[i]->overflow - allbin.bot_bin_vec[i]->overflow;
        Bin* bin;
        if(ovf_dif >= 0){
            ovf_real = (int)floor(ovf_dif);
            bin = allbin.top_bin_vec[i];
        }
        else if(ovf_dif < 0){
            ovf_real = (int)floor(-ovf_dif);
            bin = allbin.bot_bin_vec[i];
        }

        if(allbin.bin_bucket.end()==allbin.bin_bucket.find(ovf_real)){
            allbin.bin_bucket[ovf_real] = new list<Bin*>;
        }
        allbin.bin_bucket[ovf_real]->push_front(bin);
        list<instance_ptr>::iterator i_itr;
        for(i_itr=bin->inst_list.begin(); i_itr!=bin->inst_list.end(); ++i_itr){
            (*i_itr)->bin_bucket_itr = allbin.bin_bucket[ovf_real]->begin();
        }
    }
}

instance_ptr find_basecell(AllBin& allbin, Bin* bin, double area_diff, dataBase_ptr db){
    list<instance_ptr>::iterator itr;
    instance_ptr inst = nullptr;
    double area = 0;
    for(itr=bin->inst_list.begin(); itr!=bin->inst_list.end(); ++itr){
        if((double)((*itr)->area) < area_diff){
            if((double)((*itr)->area) > area){
                inst = *itr;
                area = (double)((*itr)->area);
            }
        }
    }
    return inst;
}

instance_ptr overflow_basecell(AllBin& allbin, dataBase_ptr db, Bin* bin){
    double area_diff;
    if(bin->die_num == 0){//top
        area_diff = db->dieDB->bot_die->targetArea - db->dieDB->bot_die->curArea;
    }
    else if(bin->die_num == 1){//bot
        area_diff = db->dieDB->top_die->targetArea - db->dieDB->top_die->curArea;
    }

    return find_basecell(allbin, bin, area_diff, db);
}

bool overflow_condition(AllBin& allbin, double base_overflow){
    bool top = (allbin.ovf_avg[0] <= base_overflow);
    bool bot = (allbin.ovf_avg[1] <= base_overflow);
    return top&&bot;
}

void overflow_fm(AllBin& allbin, dataBase_ptr db, GainBucket& gb, double base_overflow){
    map<int, list<Bin*>*>::iterator itr;
    list<Bin*>::iterator b_itr;
    int count = 0;
    while(!overflow_condition(allbin, base_overflow)){
        ++count;
        bool for_select_base = true;
        itr = allbin.bin_bucket.end();
        --itr;
        while(for_select_base){
            if(itr == allbin.bin_bucket.begin()){
                if(itr->second->empty()){
                    for_select_base = false;
                    break;
                }
            }
            if(!itr->second->empty()){
                b_itr = itr->second->begin();
                bool basecell = false;
                instance_ptr inst = overflow_basecell(allbin, db, *b_itr);
                if(inst) basecell = true;
                while(!basecell){
                    ++b_itr;
                    if(b_itr == itr->second->end()){
                        basecell = false;
                        break;
                    }
                    inst = overflow_basecell(allbin, db, *b_itr);
                    if(inst) basecell = true;
                }
                if(b_itr == itr->second->end()){
                    if(itr == allbin.bin_bucket.begin()){
                        for_select_base = false;
                        break;
                    }
                    else --itr;
                }
                else if(basecell){
                    ovf_mv_and_up(inst, allbin, db, gb);
                    for_select_base = false;
                }
                else --itr;        
            }
            else --itr;
        }
    }
    for_best_reset(db, gb, true, false, true);
    if(db->dieDB->top_die->curArea > db->dieDB->top_die->targetArea){
        cout<<"error: top die overflow"<<endl;
        exit(0);
    }
    else if(db->dieDB->bot_die->curArea > db->dieDB->bot_die->targetArea){
        cout<<"error: bot die overflow"<<endl;
        exit(0);
    }
}

void bin_mv_and_up(instance_ptr inst, AllBin& allbin, dataBase_ptr db, GainBucket& gb){
    double x = inst->center.x;
    double y = inst->center.y;

    before_inst_bin_update(inst, allbin, db);
    move_and_update(inst, db, gb, false, true, false, true);
    after_inst_bin_update(inst, allbin, db);

    if(x != inst->center.x){
        cout<<"error: position changed"<<endl;
        exit(0);
    }
    else if(y != inst->center.y){
        cout<<"error: position changed"<<endl;
        exit(0);
    }
}

bool bin_basecell(instance_ptr inst, dataBase_ptr db, AllBin& allbin, double base_overflow){
    bool die_util;
    bool bin_util;

    int oppArea;
    die_ptr opp_die;
    if(inst->dieNum == 1){//bot
        opp_die = db->dieDB->top_die;
    }
    else if(inst->dieNum == 0){//top
        opp_die = db->dieDB->bot_die;
    }

    cellSpec_ptr tech = inst->masterCell->cellSpec_head;
    while(tech){
        if(!strcmp(tech->techName, opp_die->techName)){
            oppArea = (tech->sizeX)*(tech->sizeY);
            break;
        }
        tech = tech->next;
    }
    
    if((opp_die->curArea + oppArea) > opp_die->targetArea) die_util = false;
    else die_util = true;

    if(allbin.ovf_avg[0] > base_overflow) bin_util = false;
    else if(allbin.ovf_avg[1] > base_overflow) bin_util = false;
    else bin_util = true;

    return die_util && bin_util;
}

void bin_bi_partition(AllBin& allbin, dataBase_ptr db, GainBucket& gb, double base_overflow){
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
                    while(!bin_basecell(*l_itr, db, allbin, base_overflow)){
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
                    else if(bin_basecell(*l_itr, db, allbin, base_overflow)){
                        //cout<<gb<<endl;
                        bin_mv_and_up(*l_itr, allbin, db, gb);
                        for_select_base = false;
                    }
                    else --itr;
                }
                else --itr;
            }
        }
        for_best_reset(db, gb, true, false, true);
        //for debugging
        if(true){
            if(!normal_path(db, gb)){
                cout<<"error: bi-partition. check the code"<<endl;
                break;
                exit(0);
            }
        }
        cout<<"path-count: "<<count<<endl;
    }
}

void bin_FM(dataBase_ptr db, int bin_num_x, int bin_num_y, double base_overflow){
    cout<<"=================Bin-based Die_Partition: start=================="<<endl;
    
    int die_num = db->instanceDB->inst_array[0]->dieNum;

    cout<<"Bin-based Die_Partition: make gain bucket start"<<endl;
    GainBucket gb;
    bin_init_db_parameter(db, gb, die_num);
    init_gainbucket(db, gb, true, false, true);
    cout<<"Bin-based Die_Partition: make gain bucket complete"<<endl;

    //init partition, 이전 fm과 동일
    cout<<"Bin-based Die_Partition: init partition start"<<endl;
    bin_init_partition(db, gb, die_num);
    print_state(db, gb);
    cout<<"Bin-based Die_Partition: init partition complete"<<endl;
    
    //bin init
    cout<<"Bin-based Die_Partition: make bin start"<<endl;
    AllBin allbin(bin_num_x, bin_num_y, db);
    init_bin(allbin, db);
    cout<<"Bin-based Die_Partition: make bin complete"<<endl;

    //bin based overflow fm
    //make bin bucket
    cout<<"Bin-based Die_Partition: make bin bucket start"<<endl;
    make_bin_bucket(allbin, db);
    cout<<"Bin-based Die_Partition: make bin bucket complete"<<endl;
    //bin based overflow fm
    cout<<"Bin-based Die_Partition: overflow fm start"<<endl;
    overflow_fm(allbin, db, gb, base_overflow);
    print_state(db, gb);
    cout<<"Bin-based Die_Partition: overflow fm complete"<<endl;

    //bin based bi partition
    cout<<"Bin-based Die_Partition: overflow fm start"<<endl;
    bin_bi_partition(allbin, db, gb, base_overflow);
    print_state(db, gb);
    
    if(end_path(db, gb)){
        //print_state(db, gb);
        //end_partition(db);
        cout<<"Bin-based Die_Partition: overflow fm complete"<<endl;
    }
    else{
        cout<<"Die_partition: error. check the code"<<endl;
        exit(0);
    }
 
    cout<<"================Bin-based Die_Partition: complete================"<<endl;
}

}