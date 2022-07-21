#include "FM/FM_Die.h"
#include "DB/InputParser.h"
#include "FM/Die_Partition.h"
#include "GP/init_placement.h"
#include "GP/opt.h"
#include "GP/wlen.h"
#include "GP/nesterov.h"
#include "LG/legalization.h"
#include "DB/ResultWriter.h"
#include <chrono>

using namespace std;

int main(int argc, char** argv){
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    dataBase_ptr DB = DataBase_init(argv[1]);
    int ns_epoch = atoi(argv[2]);
    double lr = atof(argv[3]);
    double lr_decay = atof(argv[4]);
    double momentum_X = atof(argv[5]);
    double momentum_Y = atof(argv[6]);
    double charge_weight = atof(argv[7]);
    int binX = atoi(argv[8]);
    int binY = atoi(argv[9]);
    struct POS binSize;
    binSize.x = binX;
    binSize.y = binY;
    std::chrono::system_clock::time_point db_end = std::chrono::system_clock::now();
    std::chrono::milliseconds db_const_time = std::chrono::duration_cast<std::chrono::milliseconds>(db_end-start);
    std::cout << "DB time: " << db_const_time.count() << "ms" << std::endl;

    cout<<"db complete"<<endl;
    
    FM_Die::die_partition(DB, false, true, false, false);

    printf("Cur Top die util = %3f\n", DB->dieDB->top_die->curUtil);
    printf("Cur Bot die util = %3f\n\n", DB->dieDB->bot_die->curUtil);

    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::chrono::milliseconds milliSec = std::chrono::duration_cast<std::chrono::milliseconds>(end-db_end);
    std::cout<<"FM time: "<<milliSec.count()<<"ms"<<std::endl;

    initial_placement(DB);
    std::chrono::system_clock::time_point placement_end = std::chrono::system_clock::now();
    std::chrono::milliseconds placement_time = std::chrono::duration_cast<std::chrono::milliseconds>(placement_end-end);
    std::cout<<"IP time: "<<placement_time.count()<<"ms"<<std::endl;

    fftDB_ptr fftDB_bot = create_FFTDB(DB);
    fftDB_ptr fftDB_top = create_FFTDB(DB);
    fftDB_ptr fftDB_term = create_FFTDB(DB);
    binDB_ptr binDB = createBin(DB, binSize);
    double HPWL = 0.0;
    
    std::chrono::system_clock::time_point GP_start = std::chrono::system_clock::now();
    ns_ptr ns = nesterov_init(ns_epoch, lr, lr_decay, momentum_X, momentum_Y, charge_weight);
    do_nesterov(DB, fftDB_top, fftDB_bot, fftDB_term, binDB, ns);
    
    std::chrono::system_clock::time_point GP_end = std::chrono::system_clock::now();
    std::chrono::milliseconds GP_time = std::chrono::duration_cast<std::chrono::milliseconds>(GP_end - GP_start);
    std::cout<<"GP time: "<< GP_time.count() << "ms\n";
    
    #ifdef PIPE_LG
    struct FPOS pmin;
    char filename[30];
    sprintf(filename, "../result/");
    if (!strcmp(argv[1], "./../case1.txt")) strcat(filename, "case1.txt");
    else if (!strcmp(argv[1], "./../case2.txt")) strcat(filename, "case2.txt");
    else if (!strcmp(argv[1], "./../case3.txt")) strcat(filename, "case3.txt");
    else if (!strcmp(argv[1], "./../case4.txt")) strcat(filename, "case4.txt");
    else strcat(filename, "case.txt");
    FILE* out_file = fopen(filename, "w");
    fprintf(out_file, "Top\n");
    for (int i = 0; i < DB->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = DB->instanceDB->inst_array[i];
        pmin.x = curInst->center.x - (double)curInst->sizeX * 0.5;
        pmin.y = curInst->center.y - (double)curInst->sizeY * 0.5;
        if (curInst->dieNum == 0) fprintf(out_file, "%s %3lf %3lf %d %d\n", curInst->instanceName, pmin.x, pmin.y, curInst->sizeX, curInst->sizeY);
    }
    fprintf(out_file, "Bot\n");
    for (int i = 0; i < DB->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = DB->instanceDB->inst_array[i];
        pmin.x = curInst->center.x - (double)curInst->sizeX * 0.5;
        pmin.y = curInst->center.y - (double)curInst->sizeY * 0.5;
        if (curInst->dieNum != 0) fprintf(out_file, "%s %3lf %3lf %d %d\n", curInst->instanceName, pmin.x, pmin.y, curInst->sizeX, curInst->sizeY);
    }
    fprintf(out_file, "Terminal\n");
    for (int i = 0; i < DB->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = DB->terminalDB->term_array[i];
        pmin.x = curTerm->center.x - (double)curTerm->sizeX * 0.5;
        pmin.y = curTerm->center.y - (double)curTerm->sizeY * 0.5;
        fprintf(out_file, "%s %3lf %3lf %d\n", curTerm->netName, pmin.x, pmin.y, curTerm->spacing);
    }
    fclose(out_file);
    #endif

    legalize(DB, "NULL");
    std::chrono::system_clock::time_point lg_end = std::chrono::system_clock::now();
    std::chrono::milliseconds lg_time = std::chrono::duration_cast<std::chrono::milliseconds>(lg_end-end);
    std::cout<<"LG 시간: "<<lg_time.count()<<"ms"<<std::endl;

    write(DB, argv[10]);

    printf("Destroying fftDB\n");
    destroy_FFTDB(fftDB_top);
    destroy_FFTDB(fftDB_bot);
    destroy_FFTDB(fftDB_term);
    printf("Destroying binDB\n");
    destroyBinDB(binDB);
    printf("Destroying DB\n");

    return 0;
}