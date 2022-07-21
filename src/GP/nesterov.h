#ifndef _NS_H
#define _NS_H

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "opt.h"
#include <omp.h>


struct NS_INIT
{
    int num_epoch;
    double lr;
    double lr_term;
    double lr_decay;
    struct FPOS momentum;
    double charge_weight;
}; typedef struct NS_INIT* ns_ptr;


// ns_ptr nesterov_init(void);
ns_ptr nesterov_init(int ns_epoch, double learning_rate, double lear_decay, double momentum_X, double momentum_Y, double charge_weight);
void do_nesterov(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB, ns_ptr ns);
void update_pos_grad_info(dataBase_ptr data, struct FPOS *Pos, struct FPOS *Grad, int mode);
double get_dis(struct FPOS *cur, struct FPOS *prev, int N);
struct FPOS get_lc(struct FPOS *curPOS, struct FPOS *prevPOS,
                   struct FPOS *curGrd, struct FPOS *prevGrd, int N);

#endif