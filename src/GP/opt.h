#ifndef _OPT_H
#define _OPT_H

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "charge.h"
#include "wlen.h"
#include "init_placement.h"
#include <omp.h>
#include <chrono>


void opt_init(dataBase_ptr data, fftDB_ptr fftDB, binDB_ptr binDB, struct POS binSize);
void calc_gradient(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB);
void move_instance_local(dataBase_ptr data, instance_ptr curInst, double lr, struct FPOS momentum, double charge_weight);
void move_instance_global(dataBase_ptr data, double lr, struct FPOS momentum, double charge_weight);
void move_terminal_local(dataBase_ptr data, terminal_ptr curTerm, double lr, struct FPOS momentum, double charge_weight);
void move_terminal_global(dataBase_ptr data, double lr, struct FPOS momentum, double charge_weight);
void post_movement_func(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB);
#endif