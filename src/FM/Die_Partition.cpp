#include "Die_Partition.h"

namespace FM_Die_Partition{

using namespace std;

bool basecell(FM_Instance* inst){
    if((inst->opp_Die->area + inst->area[inst->opp_Die->techName]) > inst->opp_Die->targetArea){
        return false;
    }   
    return true;
}

bool init_condition(FM& fm){
    if(fm.Die_vec[0]->area <= fm.Die_vec[0]->targetArea){
        return true;
    }
    return false;
}

void init_partition(FM& fm){
    map<int, list<FM_Instance*>*>::iterator itr;
    list<FM_Instance*>::iterator l_itr;
    int count = 0;
    while(!init_condition(fm)){
        bool for_select_base = true;
        itr = fm.GB->TGB.end();
        --itr;
        while(for_select_base){
            if(itr == fm.GB->TGB.begin()){
                if(itr->second->empty()){
                    for_select_base = false;
                    break;
                }
            }

            if(!itr->second->empty()){
                l_itr = itr->second->begin();
                while(!basecell(*l_itr)){
                    ++l_itr;
                    if(l_itr == itr->second->end()){
                        break;
                    }
                }
 
                if(l_itr == itr->second->end()){
                    if(itr == fm.GB->TGB.begin()){
                        for_select_base = false;
                        break;
                    }
                    else --itr;
                }
                else if(basecell(*l_itr)){
                    (*l_itr)->move_and_update(fm.GB, true, fm);
                    for_select_base = false;
                }
                else --itr;
            }
            else --itr;
        }
        ++count;
    }
    fm.for_best_reset();
    //for debugging
    //if(!normal_path(fm)) cout<<"error: check init part code"<<endl;
}

void bi_partition(FM& fm){
    int count = 0;
    int best = fm.Net_vec.size();
    map<int, list<FM_Instance*>*>::iterator itr;
    list<FM_Instance*>::iterator l_itr;
    while(best > fm.GB->min_cut_num){//모든 instance가 이동하고 best가 갱신되었으면 
        ++count;
        best = fm.GB->min_cut_num;
        bool path = true;
        while(path){
            bool for_select_base = true;
            itr = fm.GB->TGB.end();
            --itr;
            while(for_select_base){
                //모든 cell이 이동해서 gain bucket이 비었으면 path 종료
                if(itr == fm.GB->TGB.begin()){
                    if(itr->second->empty()){
                        for_select_base = false;
                        path = false;
                        break;
                    }
                }
                //gain bucket이 비어있지 않은 경우 base cell 선택하기
                if(!itr->second->empty()){
                    l_itr = itr->second->begin();
                    //utilization 조건 만족할 때까지 list 훑으면서 cell 찾기
                    while(!basecell(*l_itr)){
                        ++l_itr;
                        if(l_itr == itr->second->end()){
                            break; //basecell loop break
                        }
                    }
                    //cell을 찾았거나, 해당 list의 마지막에 도착 한 것.
                    if(l_itr == itr->second->end()){
                        //list의 마지막에 도착 + gain의 마지막인 경우 loop 끝내기
                        if(itr == fm.GB->TGB.begin()){
                            for_select_base = false;
                            path = false;
                            break; //for_select_base loop break
                        }
                        //list의 마지막에 도착했으므로 다음 gain으로 넘어간다
                        else --itr;
                    }
                    else if(basecell(*l_itr)){
                        (*l_itr)->move_and_update(fm.GB, false, fm);
                        for_select_base = false;
                    }
                    else --itr;
                }
                else --itr; //map에 빈 key가 있을 수 있기에 반드시 필요
            }
        }
        //1번의 path가 끝나면 다음 path를 위해서 best state로 되돌려주기
        fm.for_best_reset();
        
        //for debugging
        //cout<<"path count: "<<count<<endl;
        //cout<<"best: "<<best<<", gb min: "<<fm.GB->min_cut_num<<endl;
        //if(!normal_path(fm)){
        //    cout<<"path break. check the code"<<endl;
        //    break;
        //}
    }
}

void print_state(FM& fm){
    cout<<"----------------state-----------------"<<endl;
    cout<<"<instance>"<<endl;
    for(int i=0; i<fm.Inst_vec.size(); ++i){
        cout<<fm.Inst_vec[i];
    }
    cout<<"<net>"<<endl;
    for(int i=0; i<fm.Net_vec.size(); ++i){
        cout<<fm.Net_vec[i];
    }
    cout<<"<die>"<<endl;
    for(int i=0; i<fm.Die_vec.size(); ++i){
        cout<<fm.Die_vec[i];
    }
    cout<<fm.GB;
    cout<<"--------------------------------------"<<endl;
}

bool size_check(FM& fm){
    int die1_size = 0;
    int die2_size = 0;
    int die1_num = 0;
    int die2_num = 0;
    int die1 = fm.Die_vec[0]->area;
    int die2 = fm.Die_vec[1]->area;
    int num1 = fm.Die_vec[0]->instNum;
    int num2 = fm.Die_vec[1]->instNum;
    for(int i=0; i<fm.Inst_vec.size(); ++i){
        if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[0]){
            ++die1_num;
            die1_size += fm.Inst_vec[i]->area[fm.Inst_vec[i]->cur_Die->techName];
        }
        else if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[1]){
            ++die2_num;
            die2_size += fm.Inst_vec[i]->area[fm.Inst_vec[i]->cur_Die->techName];
        }
    }

    if(die1 != die1_size) cout<<"error: die1's size"<<endl;
    else if(die2 != die2_size) cout<<"error: die2's size"<<endl;
    else if(die1_num != num1) cout<<"error: die1's number of inst"<<endl;
    else if(die2_num != num2) cout<<"error: die2's number of inst"<<endl;
    else {
        //cout<<"no error with die size and num"<<endl;
        return true;
    }
    return false;
}

bool mincut_check(FM& fm){
    int gb_min_cut = fm.GB->min_cut_num;
    int gb_cut = fm.GB->cut_net_num;
    int real_cut = 0;
    int _real_cut = 0;
    vector<FM_Net*>::iterator itr = fm.Net_vec.begin();
    for(; itr!=fm.Net_vec.end(); ++itr){
        if((*itr)->cut) ++real_cut;
        if((*itr)->inst_dist[fm.Die_vec[0]->dieName]!=0 && (*itr)->inst_dist[fm.Die_vec[1]->dieName]!=0) ++_real_cut;
    }
    if(gb_min_cut == gb_cut){
        if(gb_min_cut == real_cut){
            //cout<<"no error with min-cut"<<endl;
            cout<<"real: "<<real_cut<<", _real: "<<_real_cut<<endl;
            return true;
        }
        else cout<<"error: real(curret) cut"<<endl;
    }
    else cout<<"error: gb cut state"<<endl;

    cout<<"real: "<<real_cut<<", _real: "<<_real_cut<<endl;

    return false;
}

void tech_check(FM& fm){
    unsigned int top_area = 0;
    unsigned int bot_area = 0;
    for(int i=0; i<fm.Inst_vec.size(); ++i){
        if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[0]){
            top_area += fm.Inst_vec[i]->area[fm.Inst_vec[i]->cur_Die->techName];
        }
        else if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[1]){
            bot_area += fm.Inst_vec[i]->area[fm.Inst_vec[i]->cur_Die->techName];
        }
    }

    if(top_area != fm.Die_vec[0]->area) cout<<"top die area wrong"<<endl;
    else if(bot_area != fm.Die_vec[1]->area) cout<<"bot die area wrong"<<endl;
    else cout<<"no error in area"<<endl;
}

void simple_print(FM& fm){
    cout<<"---------------simple state-----------------"<<endl;
    cout<<"instance: "<<fm.Inst_vec.size()<<", net: "<<fm.Net_vec.size()<<endl;
    cout<<"min cut: "<<fm.GB->min_cut_num<<", current cut: "<<fm.GB->cut_net_num<<endl;
    cout<<"die area: "<<fm.Die_vec[0]->size<<endl;
    cout<<"instance number: ("<<fm.Die_vec[0]->instNum<<", "<<fm.Die_vec[1]->instNum<<")"<<endl;
    cout<<"area of ("<<fm.Die_vec[0]->dieName<<", "<<fm.Die_vec[1]->dieName;
    cout<<") : ("<<fm.Die_vec[0]->area<<"/"<<fm.Die_vec[0]->targetArea;
    cout<<", "<<fm.Die_vec[1]->area<<"/"<<fm.Die_vec[1]->targetArea<<")";
    cout<<"=("<<(float)fm.Die_vec[0]->area/(float)fm.Die_vec[0]->size<<", "<<(float)fm.Die_vec[1]->area/(float)fm.Die_vec[1]->size<<")"<<endl;
    cout<<"--------------------------------------------"<<endl;
}

bool normal_path(FM& fm){
    bool size = size_check(fm);
    bool mincut = mincut_check(fm);

    return size&&mincut;
}

void update_DB(dataBase_ptr db, FM& fm){
    for(int i=0; i<fm.Inst_vec.size(); ++i){
        char* inst_name = (char*)fm.Inst_vec[i]->instName.data();
        instance_ptr inst_ptr = get_instance_with_name(db, inst_name);
        int is_top;
        if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[0]){
            //top
            is_top = 1;
        }
        else if(fm.Inst_vec[i]->cur_Die == fm.Die_vec[1]){
            //bot
            is_top = 0;
        }
        else cout<<"error: in update, Die name error"<<endl;
        //이 함수만 쓰면 update가 완료 되는 것인지 질문
        place_instance_in_die(db, is_top, inst_ptr);  
    }
    //for debugging
    //if(db->dieDB->top_die->curUtil != (float)fm.Die_vec[0]->area/(float)fm.Die_vec[0]->size){
    //    cout<<"update error: db util- "<<db->dieDB->top_die->curUtil<<" != ";
    //    cout<<"fm util- "<<(float)fm.Die_vec[0]->area/(float)fm.Die_vec[0]->size<<endl;
    //}
    //else if(db->dieDB->bot_die->curUtil != (float)fm.Die_vec[1]->area/(float)fm.Die_vec[1]->size){
    //    cout<<"update error: db util- "<<db->dieDB->bot_die->curUtil<<" != ";
    //    cout<<"fm util- "<<(float)fm.Die_vec[1]->area/(float)fm.Die_vec[1]->size<<endl;
    //}
    //else if(db->dieDB->top_die->targetUtil < db->dieDB->top_die->curUtil){
    //    cout<<"update error: db target util- "<<db->dieDB->top_die->targetUtil<<" < ";
    //    cout<<"fm util- "<<(float)fm.Die_vec[0]->area/(float)fm.Die_vec[0]->size<<endl;
    //}
    //else if(db->dieDB->bot_die->targetUtil < db->dieDB->bot_die->curUtil){
    //    cout<<"update error: db target util- "<<db->dieDB->bot_die->targetUtil<<" < ";
    //    cout<<"fm util- "<<(float)fm.Die_vec[1]->area/(float)fm.Die_vec[1]->size<<endl;
    //}
    //else{
    //    cout<<"update complete: target util=("<<db->dieDB->top_die->targetUtil<<", "<<db->dieDB->bot_die->targetUtil<<") > ";
    //    cout<<"cur util=("<<db->dieDB->top_die->curUtil<<", ";
    //    cout<<db->dieDB->bot_die->curUtil<<")"<<endl;
    //}
}

void Die_partition(dataBase_ptr db){
    cout<<"Die_partition: start Die_partition"<<endl;
    cout<<"Die_partition: download data from database"<<endl;
    FM fm = FM(db);
    cout<<"Die_partition: start init_partition"<<endl;
    init_partition(fm);
    cout<<"Die_partition: start bi_partition(Die_parition)"<<endl;
    bi_partition(fm);
    cout<<"Die_partition: start update result to database"<<endl;
    //tech_check(fm); //for debugging
    update_DB(db, fm);
    simple_print(fm);
    cout<<"Die_partition: end partition"<<endl;
}
}