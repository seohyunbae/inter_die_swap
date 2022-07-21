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

    top = false;
}

AllBin::AllBin(int x_num, int y_num, dataBase_ptr db){
    x_bin_num = x_num;
    y_bin_num = y_num;

    die_ptr top = db->dieDB->top_die;
    die_size_x = (double)top->upperRightX - (double)top->lowerLeftX;
    die_size_y = (double)top->upperRightY - (double)top->lowerLeftY;
    target_util = (double)top->targetUtil;

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
            bin->top = true;
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

void AllBin::cal_overflow(){

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
        Bin bin = allbin.top_bin_vec[index];
        if(bin.bin_idx != index){
            cout<<"error: top bin index count error"<<endl;
            exit(0);
        }
        bin.inst_vec.push_back(inst);
        bin.sum_inst_area += inst->area;
        ++bin.inst_num; 
        return true;
    }
    else if(inst->dieNum == 1){//bot

        return false;
    }
}

void inst_bin_update(instance_ptr inst, AllBin& allbin, dataBase_ptr db){

}

}