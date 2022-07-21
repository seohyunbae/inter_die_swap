#include "nesterov.h"


ns_ptr nesterov_init(int ns_epoch, double learning_rate, double lear_decay, double momentum_X, double momentum_Y, double charge_weight)
{
    ns_ptr my_ns = (ns_ptr)malloc(sizeof(struct NS_INIT));
    my_ns->num_epoch = ns_epoch;
    #ifdef OVFL_MODE
    my_ns->num_epoch = 500;
    #endif
    my_ns->lr = learning_rate;
    my_ns->lr_term = learning_rate;
    my_ns->lr_decay = lear_decay;
    my_ns->momentum.x = momentum_X;
    my_ns->momentum.y = momentum_Y;
    my_ns->charge_weight = charge_weight;
    return my_ns;
}


void do_nesterov(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB, ns_ptr ns)
{
    int epoch;
    struct FPOS defmoment;
    struct FPOS* instPos0;
    struct FPOS* instPos1;
    struct FPOS* instGrd0;
    struct FPOS* instGrd1;
    struct FPOS* termPos0;
    struct FPOS* termPos1;
    struct FPOS* termGrd0;
    struct FPOS* termGrd1;
    struct FPOS lc;
    struct FPOS lc_term;
    instPos0 = (struct FPOS*)calloc(sizeof(struct FPOS), data->instanceDB->numInsts);
    instPos1 = (struct FPOS*)calloc(sizeof(struct FPOS), data->instanceDB->numInsts);
    instGrd0 = (struct FPOS*)calloc(sizeof(struct FPOS), data->instanceDB->numInsts);
    instGrd1 = (struct FPOS*)calloc(sizeof(struct FPOS), data->instanceDB->numInsts);
    termPos0 = (struct FPOS*)calloc(sizeof(struct FPOS), data->terminalDB->numTerminals);
    termPos1 = (struct FPOS*)calloc(sizeof(struct FPOS), data->terminalDB->numTerminals);
    termGrd0 = (struct FPOS*)calloc(sizeof(struct FPOS), data->terminalDB->numTerminals);
    termGrd1 = (struct FPOS*)calloc(sizeof(struct FPOS), data->terminalDB->numTerminals);
    defmoment.x = defmoment.y = 1.0;
    double prev_HPWL, cur_HPWL, lr;
    #ifdef PLOT_INSTANCE
    printf("DIEDIM %d %d %d %d\n", data->dieDB->lowerLeftX, data->dieDB->upperRightX, 
                                    data->dieDB->lowerLeftY, data->dieDB->upperRightY);
    #endif
    net_update_wa(data);
    #ifdef OVFL_MODE
    int hit = 0;
    double totalInstArea = 0.0;
    for (int i = 0; i < data->instanceDB->numInsts; i++) {
        totalInstArea += (double)data->instanceDB->inst_array[i]->area;
    }
    #endif

    for (epoch = 0; epoch < ns->num_epoch; epoch++)
    {
        #ifdef PLOT_INSTANCE
        struct FPOS pmax, pmin;
        // if (epoch % 50 == 0)
        if (epoch == 0)
        {
            for (int i = 0; i < data->instanceDB->numInsts; i++)
            {
                instance_ptr curInst = data->instanceDB->inst_array[i];
                pmax.x = curInst->center.x + (double)curInst->sizeX * 0.5;
                pmax.y = curInst->center.y + (double)curInst->sizeY * 0.5;
                pmin.x = curInst->center.x - (double)curInst->sizeX * 0.5;
                pmin.y = curInst->center.y - (double)curInst->sizeY * 0.5;
                if (curInst->dieNum == 0) printf("DRAW %d %6lf %6lf %6lf %6lf TOP\n", epoch, pmin.x, pmin.y, pmax.x, pmax.y);
                else printf("DRAW %d %6lf %6lf %6lf %6lf BOT\n", epoch, pmin.x, pmin.y, pmax.x, pmax.y);
            }
        }
        #endif
        // Initial phase
        calc_gradient(data, fftDB_top, fftDB_bot, fftDB_term, binDB);
        move_instance_global(data, ns->lr, defmoment, ns->charge_weight);
        move_terminal_global(data, ns->lr_term, defmoment, ns->charge_weight);
        update_pos_grad_info(data, instPos0, instGrd0, 0);
        update_pos_grad_info(data, termPos0, termGrd0, 1);
        post_movement_func(data, fftDB_top, fftDB_bot, fftDB_term, binDB);
        if (epoch != 0)
        {
            lc = get_lc(instPos0, instPos1, instGrd0, instGrd1, data->instanceDB->numInsts);
            lc_term = get_lc(termPos0, termPos1, termGrd0, termGrd1, data->terminalDB->numTerminals);
            if (lc.y < ns->lr)
            {
                ns->lr = std::max(lc.y, ns->lr * ns->lr_decay);
                ns->lr_term = std::max(lc_term.y, ns->lr * ns->lr_decay);
            }
            ns->lr *= ns->lr_decay;
            ns->lr_term *= ns->lr_decay;
        }
        // Momentum phase
        calc_gradient(data, fftDB_top, fftDB_bot, fftDB_term, binDB);
        move_instance_global(data, ns->lr, ns->momentum, ns->charge_weight);
        move_terminal_global(data, ns->lr_term, ns->momentum, ns->charge_weight);
        update_pos_grad_info(data, instPos1, instGrd1, 0);
        update_pos_grad_info(data, termPos1, termGrd1, 1);
        post_movement_func(data, fftDB_top, fftDB_bot, fftDB_term, binDB);
        
        lc = get_lc(instPos1, instPos0, instGrd1, instGrd0, data->instanceDB->numInsts);
        lc_term = get_lc(termPos1, termPos0, termGrd1, termGrd0, data->terminalDB->numTerminals);
        if (lc.y < ns->lr)
        {
            ns->lr = std::max(lc.y, ns->lr * ns->lr_decay);
            ns->lr_term = std::max(lc_term.y, ns->lr * ns->lr_decay);
        }
        ns->lr *= ns->lr_decay;
        ns->lr_term *= ns->lr_decay;

        #ifdef NS_VERBOSE
        double HPWL = getUnscaledHPWL(data);
        printf("EPOCH %d HPWL = %6lf\n", epoch, HPWL);
        #endif

        #ifdef OVFL_MODE
        double ovfl = get_global_ovfl(data, binDB, totalInstArea);
        // printf("LR = %6lf LR_TERM = %6lf\n", ns->lr, ns->lr_term);
        printf("OVFL = %6lf\n", ovfl);
        if (epoch > 50)
        {
            if (ovfl < 0.1) break;
            if (ovfl < 0.13) hit++;
            if (hit == 10) break;
        }
        #endif
        printf("===========================================================\n");
    }
    #ifdef PLOT_INSTANCE
    struct FPOS pmax, pmin;
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = data->instanceDB->inst_array[i];
        pmax.x = curInst->center.x + (double)curInst->sizeX * 0.5;
        pmax.y = curInst->center.y + (double)curInst->sizeY * 0.5;
        pmin.x = curInst->center.x - (double)curInst->sizeX * 0.5;
        pmin.y = curInst->center.y - (double)curInst->sizeY * 0.5;
        if (curInst->dieNum == 0) printf("DRAW %d %6lf %6lf %6lf %6lf TOP\n", epoch, pmin.x, pmin.y, pmax.x, pmax.y);
        else printf("DRAW %d %6lf %6lf %6lf %6lf BOT\n", epoch, pmin.x, pmin.y, pmax.x, pmax.y);
    }
    #endif

    free(ns);
    free(instPos0);
    free(instPos1);
    free(instGrd0);
    free(instGrd1);
    free(termPos0);
    free(termPos1);
    free(termGrd0);
    free(termGrd1);
}


void update_pos_grad_info(dataBase_ptr data, struct FPOS *Pos, struct FPOS *Grad, int mode)
{
    omp_set_num_threads(NumThreads);
    if (mode == 0)
    {
        // #pragma omp parallel default(none) shared(data, Pos, Grad)
        // #pragma omp for
        for (int i = 0; i < data->instanceDB->numInsts; i++)
        {
            instance_ptr curInst = data->instanceDB->inst_array[i];
            Pos[i].x = curInst->center.x;
            Pos[i].y = curInst->center.y;
            Grad[i].x = curInst->grad.x;
            Grad[i].y = curInst->grad.y;
            // printf("Inst %3lf %3lf %3lf %3lf\n", Pos[i].x, Pos[i].y, Grad[i].x, Grad[i].y);
        }
    }
    else
    {
        // #pragma omp parallel default(none) shared(data, Pos, Grad)
        // #pragma omp for
        for (int i = 0; i < data->terminalDB->numTerminals; i++)
        {
            terminal_ptr curTerm = data->terminalDB->term_array[i];
            Pos[i].x = curTerm->center.x;
            Pos[i].y = curTerm->center.y;
            Grad[i].x = curTerm->grad.x;
            Grad[i].y = curTerm->grad.y;
            // printf("Term %3lf %3lf %3lf %3lf\n", Pos[i].x, Pos[i].y, Grad[i].x, Grad[i].y);
        }
    }
}


double get_dis(struct FPOS *cur, struct FPOS *prev, int N)
{
    double sum_dis = 0.0;
    double tmp = 0.5;
    for (int i = 0; i < N; i++)
    {
        sum_dis += (cur[i].x - prev[i].x) * (cur[i].x - prev[i].x);
        sum_dis += (cur[i].y - prev[i].y) * (cur[i].y - prev[i].y);
    }
    return std::pow(sum_dis, tmp) / (std::pow(2.0 * sum_dis, tmp));
}


struct FPOS get_lc(struct FPOS *curPOS, struct FPOS *prevPOS,
                   struct FPOS *curGrd, struct FPOS *prevGrd, int N)
{
    double dis = 0.0;
    double dnm = 0.0;
    double a;
    double lc = 0;
    struct FPOS ret_lc;
    dis = get_dis(curPOS, prevPOS, N);
    dnm = get_dis(curGrd, prevGrd, N);
    lc = dnm / dis;
    a = 1.0 / lc;
    ret_lc.x = lc;
    ret_lc.y = a;
    return ret_lc;
}