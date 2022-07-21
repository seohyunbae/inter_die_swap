#include "FM_Die.h"

using namespace std;

namespace FM_Die{

bool GainBucket::min_cut_update(){
    if(min_cut_num > cut_net_num){
        min_cut_num = cut_net_num;
        return true;
    }
    else return false;
}

GainBucket::~GainBucket(){
    map<int, list<instance_ptr>*>::iterator itr;
    for(itr=TGB.begin(); itr!=TGB.end(); ++itr){
        itr->second->clear();
        delete itr->second;
    }
}

ostream& operator<<(std::ostream& os, GainBucket& t){
    map<int, list<instance_ptr>*>::iterator itr = t.TGB.begin();
    list<instance_ptr>::iterator l_itr;
    os<<"<gain bucket>"<<endl;
    os<<"best min cut: "<<t.min_cut_num<<", now cut: "<<t.cut_net_num<<endl;
    for(;itr!=t.TGB.end();++itr){
        l_itr = itr->second->begin();
        os<<"gain: "<<itr->first<<" |";
        for(;l_itr != itr->second->end();++l_itr){
            os<<(*l_itr)->instanceName<<"("<<(*l_itr)->dieNum<<")"<<", ";
        }
        os<<endl;
    }
    return os;
}

void init_db_parameter(dataBase_ptr db, GainBucket& gb){
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

void init_gainbucket(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug){
    map<int, list<instance_ptr>*>::iterator itr;
    //차 있을 경우 초기화
    if(!gb.TGB.empty()){
        for(itr=gb.TGB.begin(); itr!=gb.TGB.end(); ++itr){
            itr->second->clear();
        }
    }
    //gain bucket 만들어주기
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        if(gb.TGB.end() == gb.TGB.find(inst->gain)){
            gb.TGB[inst->gain] = new list<instance_ptr>;
        }
        if(fifo_init){
            gb.TGB[inst->gain]->push_back(inst);
            inst->GB_itr = gb.TGB[inst->gain]->end();
            --inst->GB_itr;
        }
        else{
            gb.TGB[inst->gain]->push_front(inst);
            inst->GB_itr = gb.TGB[inst->gain]->begin();
        }
    }
}

bool for_best_reset(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug){
    //best 상태로 옮겨주기
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        for_best_die_move(db->instanceDB->inst_array[i], db, gb);
    }
    //best 상태에서 gain 계산
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        for_best_make_gain(db->instanceDB->inst_array[i], db);
    }
    //계산된 gain으로 다음 iteration위해 gainbucket 만들어주기
    init_gainbucket(db, gb, fifo_init, fifo_update, debug);
    //for debugging
    if(gb.cut_net_num == gb.min_cut_num ) return true;
    else{
        cout<<"error: reset for best is failed"<<endl;
        exit(0);
        return false;
    }
}

void move_and_update(instance_ptr inst, dataBase_ptr db, GainBucket& gb, bool init, bool fifo_init, bool fifo_update, bool debug){
    if(inst->fixed) return;

    inst->fixed = true;
    gb.TGB[inst->gain]->erase(inst->GB_itr);

    int cur_dieNum = inst->dieNum;
    int opp_dieNum;
    if(cur_dieNum == 1) opp_dieNum = 0;
    else if(cur_dieNum == 0) opp_dieNum = 1;

    inst2net_ptr i_net = inst->net_head;
    while(i_net){
        net_ptr _net = (net_ptr)i_net->net;
        if(!_net->update){
            _net->update = true;
            net2inst_ptr n_inst = _net->instance_head;
            instance_ptr _inst;
            if(_net->instance_distribution[opp_dieNum]==0){
                while(n_inst){
                    _inst = n_inst->instance;
                    if(!_inst->update){
                        _inst->update = true;
                        if(!(_inst->fixed)){
                            gb.TGB[_inst->gain]->erase(_inst->GB_itr);
                            _inst->gain += 1;
                            if(gb.TGB.end() == gb.TGB.find(_inst->gain)){
                                gb.TGB[_inst->gain] = new list<instance_ptr>;
                            }
                            if(fifo_update){
                                gb.TGB[_inst->gain]->push_back(_inst);
                                _inst->GB_itr = --gb.TGB[_inst->gain]->end();
                            }
                            else{
                                gb.TGB[_inst->gain]->push_front(_inst);
                                _inst->GB_itr = gb.TGB[_inst->gain]->begin();
                            }
                        }
                    }
                    n_inst = n_inst->next;
                }
            }
            else if(_net->instance_distribution[opp_dieNum]==1){
                while(n_inst){
                    _inst = n_inst->instance;
                    if(!_inst->update){
                        _inst->update = true;
                        if(_inst->dieNum == opp_dieNum){
                            if(!_inst->fixed){
                                gb.TGB[_inst->gain]->erase(_inst->GB_itr);
                                _inst->gain -= 1;
                                if(gb.TGB.end() == gb.TGB.find(_inst->gain)){
                                    gb.TGB[_inst->gain] = new list<instance_ptr>;
                                }
                                if(fifo_update){
                                    gb.TGB[_inst->gain]->push_back(_inst);
                                    _inst->GB_itr = --gb.TGB[_inst->gain]->end();
                                    //--_inst->GB_itr;
                                }
                                else{
                                    gb.TGB[_inst->gain]->push_front(_inst);
                                    _inst->GB_itr = gb.TGB[_inst->gain]->begin();
                                }
                            }
                        }
                    }
                    n_inst = n_inst->next;
                }
            }

            n_inst = _net->instance_head;
            while(n_inst){
                _inst = n_inst->instance;
                _inst->update = false;
                n_inst = n_inst->next;
            }

            _net->instance_distribution[cur_dieNum] -=1;
            _net->instance_distribution[opp_dieNum] +=1;

            n_inst = _net->instance_head;
            if(_net->instance_distribution[cur_dieNum]==0){
                while(n_inst){
                    _inst = n_inst->instance;
                    if(!_inst->update){
                        _inst->update = true;
                        if(!_inst->fixed){
                            gb.TGB[_inst->gain]->erase(_inst->GB_itr);
                            _inst->gain -= 1;
                            if(gb.TGB.end() == gb.TGB.find(_inst->gain)){
                                gb.TGB[_inst->gain] = new list<instance_ptr>;
                            }
                            if(fifo_update){
                                gb.TGB[_inst->gain]->push_back(_inst);
                                _inst->GB_itr = --gb.TGB[_inst->gain]->end();
                                //--_inst->GB_itr;
                            }
                            else{
                                gb.TGB[_inst->gain]->push_front(_inst);
                                _inst->GB_itr = gb.TGB[_inst->gain]->begin();
                            }
                        }
                    }
                    n_inst = n_inst->next;
                }
            }
            else if(_net->instance_distribution[cur_dieNum]==1){
                while(n_inst){
                    _inst = n_inst->instance;
                    if(!_inst->update){
                        _inst->update = true;
                        if(_inst->dieNum == cur_dieNum){
                            if(!_inst->fixed){
                                gb.TGB[_inst->gain]->erase(_inst->GB_itr);
                                _inst->gain += 1;
                                if(gb.TGB.end() ==  gb.TGB.find(_inst->gain)){
                                    gb.TGB[_inst->gain] = new list<instance_ptr>;
                                }
                                if(fifo_update){
                                    gb.TGB[_inst->gain]->push_back(_inst);
                                    _inst->GB_itr = --gb.TGB[_inst->gain]->end();
                                    //--_inst->GB_itr;
                                }
                                else{
                                    gb.TGB[_inst->gain]->push_front(_inst);
                                    _inst->GB_itr = gb.TGB[_inst->gain]->begin();
                                }
                            }
                        }
                    }
                    n_inst = n_inst->next;
                }
            }

            n_inst = _net->instance_head;
            while(n_inst){
                _inst = n_inst->instance;
                _inst->update = false;
                n_inst = n_inst->next;
            }

            //inst넘어간게 instance_distribution에 적용된 후
            if(_net->instance_distribution[0]<0 || _net->instance_distribution[1]<0){
                    cout<<"error: negative instance distribution"<<endl;
                    exit(0);
            }
            if(_net->cut){
                if(_net->instance_distribution[cur_dieNum]==0 || _net->instance_distribution[opp_dieNum]==0){
                    _net->cut = false;
                    gb.cut_net_num -= 1;
                }
            }
            else if(!_net->cut){
                if(_net->instance_distribution[cur_dieNum]>0 && _net->instance_distribution[opp_dieNum]>0){
                    _net->cut = true;
                    gb.cut_net_num += 1;
                }
            }
        }
        i_net = i_net->next;
    }

    swap_instance_to_other_die(db, inst);

    if(init){
        gb.min_cut_num = gb.cut_net_num;
        inst->bestDie = inst->dieNum;
    }
    else if(gb.min_cut_update()){
        for(int i=0; i<db->instanceDB->numInsts; ++i){
            db->instanceDB->inst_array[i]->bestDie = db->instanceDB->inst_array[i]->dieNum;
        }
    }

    i_net = inst->net_head;
    while(i_net){
        net_ptr _net = (net_ptr)i_net->net;
        _net->update = false;
        i_net = i_net->next;
    }

    return;
}

void for_best_die_move(instance_ptr inst, dataBase_ptr db, GainBucket& gb){
    inst->fixed = false;
    inst->gain = 0;

    int cur_dieNum = inst->dieNum;
    int opp_dieNum;
    if(cur_dieNum == 1) opp_dieNum = 0;
    else if(cur_dieNum == 0) opp_dieNum = 1;

    if(inst->dieNum == inst->bestDie){
        if(cur_dieNum != inst->bestDie){
            cout<<"error: best move"<<endl;
            exit(0);
        }
        return;
    }
    else{
        inst2net_ptr i_net = inst->net_head;
        while(i_net){
            net_ptr _net = (net_ptr)i_net->net;
            if(!_net->update){
                _net->update = true;
                (_net->instance_distribution[cur_dieNum]) -= 1;
                (_net->instance_distribution[opp_dieNum]) += 1;
                if(_net->instance_distribution[0]<0 || _net->instance_distribution[1]<0){
                    cout<<"error: negative instance distribution"<<endl;
                    exit(0);
                }
                if(_net->cut){
                    if(_net->instance_distribution[cur_dieNum]==0 || _net->instance_distribution[opp_dieNum]==0){
                        _net->cut = false;
                        gb.cut_net_num -= 1;
                    }
                }
                else{
                    if(_net->instance_distribution[cur_dieNum]>0 && _net->instance_distribution[opp_dieNum]>0){
                        _net->cut = true;
                        gb.cut_net_num += 1;
                    }
                }
            }
            i_net = i_net->next;
        }
        swap_instance_to_other_die(db, inst);

        i_net = inst->net_head;
        while(i_net){
            net_ptr _net = (net_ptr)i_net->net;
            _net->update = false;
            i_net = i_net->next;
        }
    }

    if(inst->dieNum != inst->bestDie){
        cout<<"error: best move"<<endl;
        exit(0);
    }

    return;
}

void for_best_make_gain(instance_ptr inst, dataBase_ptr db){
    int cur_dieNum = inst->dieNum;
    int opp_dieNum;
    if(cur_dieNum == 1) opp_dieNum = 0;
    else if(cur_dieNum == 0) opp_dieNum = 1;
    
    inst2net_ptr i_net = inst->net_head;
    while(i_net){
        net_ptr _net = (net_ptr)i_net->net;
        if(!_net->update){
            _net->update = true;
            if(_net->instance_distribution[cur_dieNum]==1) inst->gain += 1;
            if(_net->instance_distribution[opp_dieNum]==0) inst->gain -= 1;
        }
        i_net = i_net->next;
    }

    i_net = inst->net_head;
    while(i_net){
        net_ptr _net = (net_ptr)i_net->net;
        _net->update = false;
        i_net = i_net->next;
    }

    return;
}

bool basecell(instance_ptr inst, dataBase_ptr db){
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
    
    if((opp_die->curArea + oppArea) > opp_die->targetArea) return false;
    else return true;
}

bool init_condition(dataBase_ptr db){
    if(db->dieDB->top_die->curArea <= db->dieDB->top_die->targetArea) return true;
    return false;
}

void init_random(dataBase_ptr db, GainBucket& gb){
    random_device rd;
    mt19937 gen(rd());
    int total_inst = db->instanceDB->numInsts;
    uniform_int_distribution<int> dis(0, total_inst-1);
    //random하게 B로 내보내기
    while(!init_condition(db)){
        instance_ptr inst = db->instanceDB->inst_array[dis(gen)];
        move_and_update(inst, db, gb, true, true, false, true);
    }
    for_best_reset(db, gb, true, false, true);
    if(!normal_path(db, gb)){
        cout<<"error: check init part code"<<endl;
        exit(0);
    }
    return;
}

void init_partition(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug){
    map<int, list<instance_ptr>*>::iterator itr;
    list<instance_ptr>::iterator l_itr;
    int count = 0;
    while(!init_condition(db)){
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
                    move_and_update((*l_itr), db, gb, true, fifo_init, fifo_update, debug);
                    for_select_base = false;
                }
                else --itr;
            }
            else --itr;
        }
    }

    for_best_reset(db, gb, fifo_init, fifo_update, debug);
    //for debugging
    if(debug){
        if(!normal_path(db, gb)){
            cout<<"error: check init part code"<<endl;
            exit(0);
        }
    }
}

void bi_partition(dataBase_ptr db, GainBucket& gb, bool fifo_init, bool fifo_update, bool debug){
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
                    while(!basecell(*l_itr, db)){
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
                    else if(basecell(*l_itr, db)){
                        //cout<<gb<<endl;
                        move_and_update(*l_itr, db, gb, false, fifo_init, fifo_update, debug);
                        for_select_base = false;
                    }
                    else --itr;
                }
                else --itr;
            }
        }
        for_best_reset(db, gb, fifo_init, fifo_update, debug);
        //for debugging
        if(debug){
            if(!normal_path(db, gb)){
                cout<<"error: bi-partition. check the code"<<endl;
                break;
                exit(0);
            }
        }
        cout<<"path-count: "<<count<<endl;
    }
}

void die_partition(dataBase_ptr db, bool random, bool fifo_init, bool fifo_update, bool debug){
    cout<<"Die_Partition: start"<<endl;
    
    cout<<"Die_Partition: make gain bucket"<<endl;
    GainBucket gb;
    init_db_parameter(db, gb);
    init_gainbucket(db, gb, fifo_init, fifo_update, debug);
    
    if(random){
        cout<<"random init test"<<endl;
        init_random(db, gb);
        //cout<<"random init mincut: "<<gb.min_cut_num<<endl;
        print_state(db, gb);
    }
    else{
        cout<<"Die_Partition: init partition start"<<endl;
        init_partition(db, gb, fifo_init, fifo_update, debug);
        //cout<<"gain init mincut: "<<gb.min_cut_num<<endl;
        print_state(db, gb);
    }

    cout<<"Die_Partition: bi partition start"<<endl;
    bi_partition(db, gb, fifo_init, fifo_update, debug);
    //cout<<"final mincut: "<<gb.min_cut_num<<endl;

    print_state(db, gb);

    if(debug){
        if(end_path(db, gb)){
            //print_state(db, gb);
            //end_partition(db);
        }
        else{
            cout<<"Die_partition: error. check the code"<<endl;
            exit(0);
        }
    }

    return;
}

void end_partition(dataBase_ptr db){
    //for(int i=0; i<db->netDB->numNets; ++i){
    //    db->netDB->net_array[i]->instance_distribution.clear();
    //}
}

void print_state(dataBase_ptr db, GainBucket& gb){
    cout<<"------------------Die Partition State-----------------------"<<endl;
    cout<<"instance: "<<db->instanceDB->numInsts<<", net: "<<db->netDB->numNets<<endl;
    cout<<"min cut: "<<gb.min_cut_num<<", current cut: "<<gb.cut_net_num<<endl;
    cout<<"die area: "<<db->dieDB->top_die->dieArea<<endl;
    cout<<"inst num: ("<<db->dieDB->top_die->numInsts<<", "<<db->dieDB->bot_die->numInsts<<")"<<endl;
    cout<<"current area/target area: top-("<<db->dieDB->top_die->curArea<<"/"<<db->dieDB->top_die->targetArea;
    cout<<"), bot-("<<db->dieDB->bot_die->curArea<<"/"<<db->dieDB->bot_die->targetArea<<")"<<endl;
    cout<<"current util/target util: top-("<<db->dieDB->top_die->curUtil<<"/"<<(double)db->dieDB->top_die->targetUtil/(double)100;
    cout<<"), bot-("<<db->dieDB->bot_die->curUtil<<"/"<<(double)db->dieDB->bot_die->targetUtil/(double)100<<")"<<endl;
    cout<<"------------------------------------------------------------"<<endl;
}

bool size_check(dataBase_ptr db, GainBucket& gb){
    int top_area = 0;
    int bot_area = 0;
    int top_num = 0;
    int bot_num = 0;
    int _top_area = db->dieDB->top_die->curArea;
    int _bot_area = db->dieDB->bot_die->curArea;
    int _top_num = db->dieDB->top_die->numInsts;
    int _bot_num = db->dieDB->bot_die->numInsts;

    instance_ptr inst;
    for(int i=0; i<db->instanceDB->numInsts; ++i){
        inst = db->instanceDB->inst_array[i];
        if(inst->dieNum == 0){//top
            ++top_num;
            top_area += inst->area;
        }
        else if(inst->dieNum ==1){//bot
            ++bot_num;
            bot_area += inst->area;
        }
    }

    if(top_area != _top_area) cout<<"error: top die's current area"<<endl;
    else if(bot_area != _bot_area) cout<<"error: bot die's current area"<<endl;
    else if(top_num != _top_num) cout<<"error: top die's current num"<<endl;
    else if(bot_num != _bot_num) cout<<"error: bot die's current num"<<endl;
    else return true;
    exit(0);
    return false;
}

bool mincut_check(dataBase_ptr db, GainBucket& gb){
    int gb_min_cut = gb.min_cut_num;
    int gb_cut = gb.cut_net_num;
    int real_cut = 0;
    int _real_cut = 0;
    int __real_cut = 0;
    int neg_inst = 0;
    int inst_dist_0 = 0;
    int inst_dist_1 = 0;

    for(int i=0; i<db->netDB->numNets; ++i){
        net_ptr net = db->netDB->net_array[i];
        if(net->cut) ++real_cut;
        if(net->instance_distribution[0]>0 && net->instance_distribution[1]>0) ++_real_cut;
        if(net->instance_distribution[0]<0 || net->instance_distribution[1]<0){
            ++neg_inst;
        }
        net2inst_ptr inst = net->instance_head;
        inst_dist_0 = 0;
        inst_dist_1 = 0;
        while(inst){
            instance_ptr _inst = inst->instance;
            if(!_inst->update){
                _inst->update = true;
                if(_inst->dieNum == 0) ++inst_dist_0;
                else if(_inst->dieNum == 1) ++inst_dist_1;
            }
            inst = inst->next;
        }
        if(inst_dist_0>0 && inst_dist_1>0) ++__real_cut;
        
        if(inst_dist_0 != net->instance_distribution[0]){
            cout<<"error: net's instance distribution[0], netName="<<net->netName<<endl;
            exit(0);
        }
        else if(inst_dist_1 != net->instance_distribution[1]){
            cout<<"error: net's instance distribution[1], netName="<<net->netName<<endl;
            exit(0);
        }

        inst = net->instance_head;
        while(inst){
            instance_ptr _inst = inst->instance;
            _inst->update = false;
            inst = inst->next;
        }
    }

    if(gb_min_cut != gb_cut) cout<<"error: in gain bucket, min_cut_num != cut_net_num"<<endl;
    else if(gb_min_cut != real_cut) cout<<"error: min-cut / net has wrong boolean(cut)"<<endl;
    else if(gb_min_cut != _real_cut) cout<<"error: net's inst_distribution error"<<endl;
    else if(gb_min_cut != __real_cut) cout<<"error: min cut is different real cut"<<endl;
    else if(neg_inst > 0) cout<<"error: neg_inst= "<<neg_inst<<", negative number of instance"<<endl;
    else {
        return true;
    }
    exit(0);

    return false;
}

bool normal_path(dataBase_ptr db, GainBucket& gb){
    return size_check(db, gb) && mincut_check(db, gb);
}

bool util_check(dataBase_ptr db, GainBucket& gb){
    double _top_util = db->dieDB->top_die->curUtil;
    double _bot_util = db->dieDB->bot_die->curUtil;
    double top_target = db->dieDB->top_die->targetUtil;
    double bot_target = db->dieDB->bot_die->targetUtil;
    double top_util = (double)db->dieDB->top_die->curArea/(double)db->dieDB->top_die->dieArea;
    double bot_util = (double)db->dieDB->bot_die->curArea/(double)db->dieDB->bot_die->dieArea;

    if(top_util!=_top_util || bot_util!=_bot_util){
        cout<<"error: utilization not equal"<<endl;
        cout<<"top/_top = "<<top_util<<"/"<<_top_util;
        cout<<", bot/_bot = "<<bot_util<<"/"<<_bot_util<<endl;;
    }
    else if(top_util>=top_target || bot_util>=bot_target) cout<<"error: utilization overflow"<<endl;
    else return true;
    exit(0);
    return false;
}

bool end_path(dataBase_ptr db, GainBucket& gb){
    return normal_path(db, gb) && util_check(db, gb);
}

}