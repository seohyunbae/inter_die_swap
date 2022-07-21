#ifndef _CHARGE_H
#define _CHARGE_H

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "bin.h"
#include "fft.h"
#include "utils.h"
#include <omp.h>


void generate_global_density(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term);
void generate_instance_density(instance_ptr curInst, fftDB_ptr fftDB);
void generate_terminal_density(terminal_ptr curTerm, fftDB_ptr fftDB);
void reset_global_density(fftDB_ptr fftDB);
void potn_grad_2d_global(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, binDB_ptr binDB);
void potn_grad_2d_global_term(dataBase_ptr data, fftDB_ptr fftDB_term, binDB_ptr binDB);
void potn_grad_2D_local(instance_ptr curInst, double* cellLambda, fftDB_ptr fftDB, binDB_ptr binDB);
void potn_grad_2d_local_term(terminal_ptr curTerm, double* cellLambda, fftDB_ptr fftDB, binDB_ptr binDB);


#endif