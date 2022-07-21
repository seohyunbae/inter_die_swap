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
    if(inst->dieNum == 0){//top
        Bin* bin = allbin.top_bin_vec[index];
        if(bin->bin_idx != index){
            cout<<"error: top bin index count error"<<endl;
            exit(0);
        }
        bin->inst_vec.push_back(inst);
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
        bin->inst_vec.push_back(inst);
        bin->sum_inst_area += inst->area;
        ++bin->inst_num;
        return false;
    }
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
            bin->overflow = bin->cur_util - allbin.target_util[bin->die_num];
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
            bin->overflow = bin->cur_util - allbin.target_util[bin->die_num];
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

void inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db){

}

}