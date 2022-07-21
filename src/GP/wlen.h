#ifndef _WLEN_H
#define _WLEN_H

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "utils.h"
#include <omp.h>

void update_net_pos(dataBase_ptr data);
double getUnscaledHPWL(dataBase_ptr data);
void net_update_wa(dataBase_ptr data);
struct FPOS get_instance_net_wlen_grad_wa(struct FCOORD obj, net_ptr curNet, net2inst_ptr pin);
struct FPOS get_terminal_net_wlen_grad_wa(struct FCOORD obj, net_ptr curNet, terminal_ptr pin);
void instance_wlen_grad_wa(instance_ptr curInst);
void terminal_wlen_grad_wa(terminal_ptr curTerm);
void update_global_wlen_grad_wa(dataBase_ptr data);

#endif