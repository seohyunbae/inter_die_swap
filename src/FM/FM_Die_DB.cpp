#include "FM_Die_DB.h"

namespace FM_Die_Partition{
using namespace std;

//FM
FM::FM(dataBase_ptr db){
    //die
    Die_vec.push_back(new FM_Die(db->dieDB->top_die));
    Die_vec.push_back(new FM_Die(db->dieDB->bot_die));
    //instance
    //instance내의 net_vec update는 FM의 net_vec형성 이후에 진행.
    if(db->instanceDB->numInsts != 0){
        for(int i=0; i<db->instanceDB->numInsts; ++i){
            string inst_name = string(db->instanceDB->inst_array[i]->instanceName);
            instance_ptr inst = db->instanceDB->inst_array[i];
            Inst_hash[inst_name] = new FM_Instance(inst, db, this);
            Inst_vec.push_back(Inst_hash[inst_name]);
            //top die에 면적 올인. top die's dieNum = dies vector's index = 0
            Die_vec[0]->area += Inst_hash[inst_name]->area[Inst_hash[inst_name]->cur_Die->techName];
            ++Die_vec[0]->instNum;
        }
    }
    //net
    //net_vec 형성하면서 동시에 inst와 net link
    if(db->netDB->numNets != 0){
        for(int i=0; i<db->netDB->numNets; ++i){
            string net_name = string(db->netDB->net_array[i]->netName);
            int inst_num = db->netDB->net_array[i]->numInst;
            //Net_hash.insert(make_pair(net_name, new FM_Net(db->netDB->net_array[i], this, inst_num)));
            Net_hash[net_name] = new FM_Net(db->netDB->net_array[i], this, inst_num);
            Net_vec.push_back(Net_hash[net_name]);
        }
    }

    /////make net list in instance obj
    if(db->instanceDB->numInsts != 0){
        for(int i=0; i<db->instanceDB->numInsts; ++i){
            instance_ptr inst = db->instanceDB->inst_array[i];
            string inst_name = string(inst->instanceName);
            inst2net_ptr link_ptr = inst->net_head;
            string net_name;
            while(link_ptr){
                net_ptr net = (net_ptr)link_ptr->net;
                net_name = string(net->netName);
                Inst_hash[inst_name]->Net_vec.push_back(Net_hash[net_name]);
                link_ptr = link_ptr->next;
            }
            Inst_hash[inst_name]->gain = -(Inst_hash[inst_name]->Net_vec.size()); //first gain init
        }
    }
    ////
    //make inst_vec in FM_Net, make net_vec in FM_Instance
    Inst_hash.clear();
    Net_hash.clear();
    inst_num = Inst_vec.size();
    net_num = Net_vec.size();
    die_num = Die_vec.size();
    tech_num = db->masterCellDB->numTechs;

    //gain bucket
    init_gainbucket();
}

FM::~FM(){
    if(!Inst_vec.empty()){
        for(int i=0; i<Inst_vec.size(); ++i){
            delete Inst_vec[i];
        }
    }
    if(!Net_vec.empty()){
        for(int i=0; i<Net_vec.size(); ++i){
            delete Net_vec[i];
        }
    }
    if(!Die_vec.empty()){
        for(int i=0; i<Die_vec.size(); ++i){
            delete Die_vec[i];
        }
    }
    //GainBucket은 GainBucket의 destructor에서 free
    delete GB;
}

//모든 data structure 다 완성 된 후에 gain bucket 만들기. (gain이 계산 되어 있어야함)
void FM::init_gainbucket(){
    map<int, list<FM_Instance*>*>::iterator itr;
    GB = new FM_GainBucket();
    if(!GB->TGB.empty()){
        cout<<"not empty"<<endl;//for test
        for(itr = GB->TGB.begin(); itr != GB->TGB.end(); ++itr){
            itr->second->clear();
        }
    }

    for(int i=0; i<Inst_vec.size(); ++i){
        //처음에는 넘어가면 연결된 net이 다 cut되니까 gain이 -net갯수임
        //Inst_vec[i]->gain = -(Inst_vec[i]->Net_vec.size()); //first gain init
        if(GB->TGB.end() == GB->TGB.find(Inst_vec[i]->gain)){
            GB->TGB[Inst_vec[i]->gain] = new list<FM_Instance*>;
        }
        GB->TGB[Inst_vec[i]->gain]->push_front(Inst_vec[i]);
        Inst_vec[i]->GB_itr = GB->TGB[Inst_vec[i]->gain]->begin();    
    }
}

bool FM::for_best_reset(){
    //instance의 Die랑 gain reset
    for(int i=0; i<Inst_vec.size(); ++i){
        Inst_vec[i]->for_best_Die_move(GB);
    }

    for(int i=0; i<Inst_vec.size(); ++i){
        Inst_vec[i]->for_best_make_gain();
    }

    //reset된 gain바탕으로 gainbucket다시 만들어주기
    map<int, list<FM_Instance*>*>::iterator itr;
    if(!GB->TGB.empty()){
        for(itr = GB->TGB.begin(); itr != GB->TGB.end(); ++itr){
            itr->second->clear();
        }
    }
    for(int i=0; i<Inst_vec.size(); ++i){
        if(GB->TGB.end() == GB->TGB.find(Inst_vec[i]->gain)){
            GB->TGB[Inst_vec[i]->gain] = new list<FM_Instance*>;
        }
        GB->TGB[Inst_vec[i]->gain]->push_front(Inst_vec[i]);
        Inst_vec[i]->GB_itr = GB->TGB[Inst_vec[i]->gain]->begin();
    }

    if(GB->cut_net_num == GB->min_cut_num) return true;
    else{
        cout<<"reset for best failed"<<endl;
        return false;
    }
    return false;
}

//Instance

FM_Instance::FM_Instance(instance_ptr inst, dataBase_ptr db, FM* fm){
    instName = string(inst->instanceName);
    //master cell 이용해서 area update
    cellSpec_ptr mc_ptr = inst->masterCell->cellSpec_head;
    while(mc_ptr){
        string tech_name = string(mc_ptr->techName);
        int tech_area = mc_ptr->sizeX * mc_ptr->sizeY;
        area.insert(make_pair(tech_name, tech_area));
        mc_ptr = mc_ptr->next;
    }

    gain = 0;
    fixed = false;
    //처음에는 다 top에 넣어주기
    cur_Die = fm->Die_vec[0]; //top
    opp_Die = fm->Die_vec[1]; //bottom
    best_Die = cur_Die;
}

ostream& operator<<(std::ostream& os, FM_Instance* t){
    os<<"<"<<t->instName<<"> gain: "<<t->gain<<", fixed: "<<t->fixed;
    unordered_map<string, int>::iterator itr = t->area.begin();
    os<<", area: ("<<itr->second<<", "<<(++itr)->second<<")";
    os<<", die: "<<t->cur_Die->dieName<<"-"<<t->cur_Die->techName;
    os<<", best_die: "<<t->best_Die->dieName;
    os<<", net number: "<<t->Net_vec.size()<<endl;

    return os;
}

void FM_Instance::move_and_update(FM_GainBucket* gb, bool init, FM& fm){
    if(fixed) return;

    fixed = true;
    gb->TGB[gain]->erase(GB_itr);

    for(int i=0; i<Net_vec.size(); ++i){
        int before_gain;
        if(Net_vec[i]->inst_dist[opp_Die->dieName]==0){
            for(int j=0; j<Net_vec[i]->Inst_vec.size(); ++j){
                FM_Instance* inst = Net_vec[i]->Inst_vec[j];
                if(!inst->fixed){
                    gb->TGB[inst->gain]->erase(inst->GB_itr);
                    inst->gain += 1;
                    if(gb->TGB.end() == gb->TGB.find(inst->gain)){
                        gb->TGB[inst->gain] = new list<FM_Instance*>;
                    }
                    gb->TGB[inst->gain]->push_front(inst);
                    inst->GB_itr = gb->TGB[inst->gain]->begin();
                }
            }
        }
        else if (Net_vec[i]->inst_dist[opp_Die->dieName]==1){
            for(int j=0; j<Net_vec[i]->Inst_vec.size(); ++j){
                FM_Instance* inst = Net_vec[i]->Inst_vec[j];
                if(inst->cur_Die == opp_Die){
                    if(!inst->fixed){
                        gb->TGB[inst->gain]->erase(inst->GB_itr);
                        inst->gain -= 1;
                        if(gb->TGB.end() == gb->TGB.find(inst->gain)){
                            gb->TGB[inst->gain] = new list<FM_Instance*>;
                        }
                        gb->TGB[inst->gain]->push_front(inst);
                        inst->GB_itr = gb->TGB[inst->gain]->begin();
                    }
                }
            }
        }

        Net_vec[i]->inst_dist[cur_Die->dieName] -= 1;
        Net_vec[i]->inst_dist[opp_Die->dieName] += 1;

        if(Net_vec[i]->inst_dist[cur_Die->dieName]==0){
            for(int j=0; j<Net_vec[i]->Inst_vec.size(); ++j){
                FM_Instance* inst = Net_vec[i]->Inst_vec[j];
                if(!inst->fixed){
                    gb->TGB[inst->gain]->erase(inst->GB_itr);
                    inst->gain -= 1;
                    if(gb->TGB.end() == gb->TGB.find(inst->gain)){
                        gb->TGB[inst->gain] = new list<FM_Instance*>;
                    }
                    gb->TGB[inst->gain]->push_front(inst);
                    inst->GB_itr = gb->TGB[inst->gain]->begin();
                }
            }
        }
        else if(Net_vec[i]->inst_dist[cur_Die->dieName]==1){
            for(int j=0; j<Net_vec[i]->Inst_vec.size(); ++j){
                FM_Instance* inst = Net_vec[i]->Inst_vec[j];
                if(inst->cur_Die == cur_Die){
                    if(!inst->fixed){
                        gb->TGB[inst->gain]->erase(inst->GB_itr);
                        inst->gain += 1;
                        if(gb->TGB.end() == gb->TGB.find(inst->gain)){
                            gb->TGB[inst->gain] = new list<FM_Instance*>;
                        }
                        gb->TGB[inst->gain]->push_front(inst);
                        inst->GB_itr = gb->TGB[inst->gain]->begin();
                    }
                }
            }
        }
        //cut, cut_net_num update
        if(Net_vec[i]->cut){//cut이 되어 있는 경우
            if((Net_vec[i]->inst_dist[cur_Die->dieName]==0)||(Net_vec[i]->inst_dist[opp_Die->dieName]==0)){
                Net_vec[i]->cut = false;
                gb->cut_net_num -= 1;
            }
        }
        else{//cut이 안되어 있는 경우
            if((Net_vec[i]->inst_dist[cur_Die->dieName]!=0)&&(Net_vec[i]->inst_dist[opp_Die->dieName]!=0)){
                Net_vec[i]->cut = true;
                gb->cut_net_num += 1;
            }
        }
    }
    //Die update
    cur_Die->area -= area[cur_Die->techName];
    opp_Die->area += area[opp_Die->techName];
    --cur_Die->instNum;
    ++opp_Die->instNum;

    FM_Die* temp = cur_Die;
    cur_Die = opp_Die;
    opp_Die = temp;

    //gain bucket and best_Die update
    //이전 fm에서 set_best의 기능 대체
    if(init){//init일 때는 cut수가 증가하므로 따로 update해줘야 한다.
        gb->min_cut_num = gb->cut_net_num;
        best_Die = cur_Die;
    }
    else if(gb->min_cut_update()){
        //옮겨진 instance 뿐 아니라, 전체 instance의 현재 위치가 best에 저장되어야 한다.
        for(int i=0; i<fm.Inst_vec.size(); ++i){
            fm.Inst_vec[i]->best_Die = fm.Inst_vec[i]->cur_Die;
        }
    }
    return;
}

void FM_Instance::for_best_Die_move(FM_GainBucket* gb){
    //gain update만 안해주고 나머진 싹 update
    fixed = false;
    gain = 0;

    if(cur_Die == best_Die) return;
    else{
        //net state update
        for(int i=0; i<Net_vec.size(); ++i){
            //net의 나눠져 있는 instance 갯수 update
            Net_vec[i]->inst_dist[cur_Die->dieName] -= 1;
            Net_vec[i]->inst_dist[opp_Die->dieName] += 1;
            //net state및 cut net의 수 update
            if(Net_vec[i]->cut){//cut이 되어 있는 경우
                if((Net_vec[i]->inst_dist[cur_Die->dieName]==0)||(Net_vec[i]->inst_dist[opp_Die->dieName]==0)){
                    Net_vec[i]->cut = false;
                    gb->cut_net_num -= 1;
                }
            }
            else{//cut이 안되어 있는 경우
                if((Net_vec[i]->inst_dist[cur_Die->dieName]!=0)||(Net_vec[i]->inst_dist[opp_Die->dieName]!=0)){
                    Net_vec[i]->cut = true;
                    gb->cut_net_num += 1;
                }
            }
        }
        //Die update
        cur_Die->area -= area[cur_Die->techName];
        opp_Die->area += area[opp_Die->techName];
        --cur_Die->instNum;
        ++opp_Die->instNum;

        FM_Die* temp = cur_Die;
        cur_Die = opp_Die;
        opp_Die = temp;
    }
    return;
}

void FM_Instance::for_best_make_gain(){
    for(int i=0; i<Net_vec.size(); ++i){
        if(Net_vec[i]->inst_dist[cur_Die->dieName]==1) ++gain;
        if(Net_vec[i]->inst_dist[opp_Die->dieName]==0) --gain;
    }
    return;
}

//Net

FM_Net::FM_Net(net_ptr net, FM* fm, int inst_num){
    netName = string(net->netName);
    cut = false;
    //for make inst_dist vector
    inst_dist[fm->Die_vec[0]->dieName] = inst_num; //inst_dist[top die's name] for top
    inst_dist[fm->Die_vec[1]->dieName] = 0; //inst_dist[bot die's name] for bot
    //net, instance link
    net2inst_ptr link_ptr = net->instance_head;
    string inst_name;
    while(link_ptr){
        inst_name = string(link_ptr->instanceName);
        Inst_vec.push_back(fm->Inst_hash[inst_name]);
        //fm->Inst_hash[inst_name]->Net_vec.push_back(this);
        link_ptr = link_ptr->next;
    }
}

ostream& operator<<(std::ostream& os, FM_Net* t){
    os<<"<"<<t->netName<<"> cut: "<<t->cut;
    unordered_map<string, int>::iterator itr = t->inst_dist.begin();
    os<<", inst_dist: ("<<itr->second<<", "<<(++itr)->second;
    os<<"), inst number: "<<t->Inst_vec.size()<<endl;
    return os;
}

//GainBucket
ostream& operator<<(std::ostream& os, FM_GainBucket* t){
    map<int, list<FM_Instance*>*>::iterator itr = t->TGB.begin();
    list<FM_Instance*>::iterator l_itr;
    os<<"<gain bucket>"<<endl;
    os<<"best min cut: "<<t->min_cut_num<<", now cut: "<<t->cut_net_num<<endl;
    for(;itr!=t->TGB.end();++itr){
        //l_itr = itr->second->begin();
        //for(;l_itr != itr->second->end();++l_itr){
        //    os<<(*l_itr);
        //}
        l_itr = itr->second->begin();
        os<<"gain: "<<itr->first<<" |";
        for(;l_itr != itr->second->end();++l_itr){
            os<<(*l_itr)->instName<<"("<<(*l_itr)->cur_Die->dieName<<")"<<", ";
        }
        os<<endl;
    }
    return os;
}

FM_GainBucket::~FM_GainBucket(){
    map<int, list<FM_Instance*>*>::iterator itr;
    for(itr = TGB.begin(); itr != TGB.end(); ++itr){
        itr->second->clear();
        delete itr->second;
    }
}

bool FM_GainBucket::min_cut_update(){
    if(min_cut_num > cut_net_num){
        min_cut_num = cut_net_num;
        return true;
    }
    else return false;
}

//Die

FM_Die::FM_Die(die_ptr die){
    dieName = string(die->dieName);
    techName = string(die->techName);
    instNum = 0;
    size = die->dieArea;
    //cout<<"size: "<<size<<", targetUtil: "<<die->targetUtil<<endl;
    targetArea = (float)die->dieArea * (float)die->targetUtil / (float)100;
    area = 0;
}

ostream& operator<<(std::ostream& os, const FM_Die* t){
    os<<"<"<<t->dieName<<">"<<" tech: "<<t->techName;
    os<<", size: "<<t->size<<", target: "<<t->targetArea<<", area: "<<t->area<<", inst num: "<<t->instNum<<endl;
    return os;
}

}