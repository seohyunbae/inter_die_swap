#include "wlen.h"


void update_net_pos(dataBase_ptr data)
{
    using namespace std;
    #pragma omp parallel default(none) shared(data)
    #pragma omp for
    for (int i = 0; i < data->netDB->numNets; i++)
    {
        net_ptr curNet = data->netDB->net_array[i];
        curNet->pmin_top.x = curNet->pmin_top.y = curNet->pmin_bot.x = curNet->pmin_bot.y = INFINITY;
        curNet->pmax_top.x = curNet->pmax_top.y = curNet->pmax_bot.x = curNet->pmax_bot.y = -INFINITY;
        net2inst_ptr sweep_netLink = curNet->instance_head;
        while (sweep_netLink)
        {
            FCOORD curCoord = sweep_netLink->curCoord;
            if (sweep_netLink->instance->dieNum == 0) // top Die
            {
                curNet->pmin_top.x = min(curNet->pmin_top.x, curCoord.x);
                curNet->pmin_top.y = min(curNet->pmin_top.y, curCoord.y);
                curNet->pmax_top.x = max(curNet->pmax_top.x, curCoord.x);
                curNet->pmax_top.y = max(curNet->pmax_top.y, curCoord.y);
            }
            else    // Bot die
            {
                curNet->pmin_bot.x = min(curNet->pmin_bot.x, curCoord.x);
                curNet->pmin_bot.y = min(curNet->pmin_bot.y, curCoord.y);
                curNet->pmax_bot.x = max(curNet->pmax_bot.x, curCoord.x);
                curNet->pmax_bot.y = max(curNet->pmax_bot.y, curCoord.y);
            }
            sweep_netLink = sweep_netLink->next;
        }
        if (curNet->cut == true)
        {
            terminal_ptr curTerm = (terminal_ptr)curNet->terminal;
            if (curTerm)
            {
                FCOORD curCoord = curTerm->center;
                curNet->pmin_top.x = min(curNet->pmin_top.x, curCoord.x);
                curNet->pmin_top.y = min(curNet->pmin_top.y, curCoord.y);
                curNet->pmax_top.x = max(curNet->pmax_top.x, curCoord.x);
                curNet->pmax_top.y = max(curNet->pmax_top.y, curCoord.y);
                curNet->pmin_bot.x = min(curNet->pmin_bot.x, curCoord.x);
                curNet->pmin_bot.y = min(curNet->pmin_bot.y, curCoord.y);
                curNet->pmax_bot.x = max(curNet->pmax_bot.x, curCoord.x);
                curNet->pmax_bot.y = max(curNet->pmax_bot.y, curCoord.y);
            }
        }
    }
}


double getUnscaledHPWL(dataBase_ptr data)
{
    double unscaledHPWL = 0.0;
    net_ptr curNet;
    for (int i = 0; i < data->netDB->numNets; i++)
    {
        curNet = data->netDB->net_array[i];
        if (curNet->pmin_top.x != INFINITY){
            unscaledHPWL += curNet->pmax_top.x - curNet->pmin_top.x;
            unscaledHPWL += curNet->pmax_top.y - curNet->pmin_top.y;
        }
        if (curNet->pmin_bot.x != INFINITY){
            unscaledHPWL += curNet->pmax_bot.x - curNet->pmin_bot.x;
            unscaledHPWL += curNet->pmax_bot.y - curNet->pmin_bot.y;
        }
    }
    return unscaledHPWL;
}


void net_update_wa(dataBase_ptr data)
{
    FPOS net_wlen;
    int i = 0;

    #pragma omp parallel default(none) shared(data) private(i)
    #pragma omp for
    for (i = 0; i < data->netDB->numNets; i++)
    {
        struct FPOS sum_num1_top;
        struct FPOS sum_num1_bot;
        struct FPOS sum_num2_top;
        struct FPOS sum_num2_bot;

        struct FPOS sum_denom1_top;
        struct FPOS sum_denom1_bot;
        struct FPOS sum_denom2_top;
        struct FPOS sum_denom2_bot;
        sum_num1_top.x = sum_num1_top.y = sum_num1_bot.x = sum_num1_bot.y = 0.0;
        sum_num2_top.x = sum_num2_top.y = sum_num2_bot.x = sum_num2_bot.y = 0.0;
        sum_denom1_top.x = sum_denom1_top.y = sum_denom1_bot.x = sum_denom1_bot.y = 0.0;
        sum_denom2_top.x = sum_denom2_top.y = sum_denom2_bot.x = sum_denom2_bot.y = 0.0;
        net_ptr curNet = data->netDB->net_array[i];
        for (int j = 0; j < curNet->numPins; j++)
        {
            net2inst_ptr pin = curNet->instance_head;
            FPOS fp;
            double exp_max_x, exp_min_x, exp_max_y, exp_min_y;
            fp.x = pin->curCoord.x;
            fp.x = pin->curCoord.y;
            if (pin->instance->dieNum == 0) //Top die pin
            {
                exp_max_x = (fp.x - curNet->pmax_top.x) * wlen_cof_x;
                exp_min_x = (curNet->pmin_top.x - fp.x) * wlen_cof_x;
                exp_max_y = (fp.y - curNet->pmax_top.y) * wlen_cof_y;
                exp_min_y = (curNet->pmin_top.y - fp.y) * wlen_cof_y;
                // printf("%3f, %3f, %3f, %3f\n", exp_max_x, exp_max_y, exp_min_x, exp_min_y);
                if (exp_max_x > NEG_MAX_EXP)
                {
                    pin->e1.x = fastExp(exp_max_x);
                    sum_num1_top.x += fp.x * pin->e1.x;
                    sum_denom1_top.x += pin->e1.x;
                    pin->flg1.x = 1;
                }
                else {
                    pin->flg1.x = 0;
                }
                if (exp_min_x > NEG_MAX_EXP) {
                    pin->e2.x = fastExp(exp_min_x);
                    sum_num2_top.x += fp.x * pin->e2.x;
                    sum_denom2_top.x += pin->e2.x;
                    pin->flg2.x = 1;
                }
                else pin->flg2.x = 0;

                if (exp_max_y > NEG_MAX_EXP) {
                    pin->e1.y = fastExp(exp_max_y);
                    sum_num1_top.y += fp.y * pin->e1.y;
                    sum_denom1_top.y += pin->e1.y;
                    pin->flg1.y = 1;
                }
                else pin->flg1.y = 0;

                if (exp_min_y > NEG_MAX_EXP) {
                    pin->e2.y = fastExp(exp_min_y);
                    sum_num2_top.y += fp.y * pin->e2.y;
                    sum_denom2_top.y += pin->e2.y;
                    pin->flg2.y = 1;
                }
                else pin->flg2.y = 0;
            }
            else    // bot die
            {
                exp_max_x = (fp.x - curNet->pmax_bot.x) * wlen_cof_x;
                exp_min_x = (curNet->pmin_bot.x - fp.x) * wlen_cof_x;
                exp_max_y = (fp.y - curNet->pmax_bot.y) * wlen_cof_y;
                exp_min_y = (curNet->pmin_bot.y - fp.y) * wlen_cof_y;

                if (exp_max_x > NEG_MAX_EXP)
                {
                    pin->e1.x = fastExp(exp_max_x);
                    sum_num1_bot.x += fp.x * pin->e1.x;
                    sum_denom1_bot.x += pin->e1.x;
                    pin->flg1.x = 1;
                }
                else {
                    pin->flg1.x = 0;
                }
                if (exp_min_x > NEG_MAX_EXP) {
                    pin->e2.x = fastExp(exp_min_x);
                    sum_num2_bot.x += fp.x * pin->e2.x;
                    sum_denom2_bot.x += pin->e2.x;
                    pin->flg2.x = 1;
                }
                else pin->flg2.x = 0;

                if (exp_max_y > NEG_MAX_EXP) {
                    pin->e1.y = fastExp(exp_max_y);
                    sum_num1_bot.y += fp.y * pin->e1.y;
                    sum_denom1_bot.y += pin->e1.y;
                    pin->flg1.y = 1;
                }
                else pin->flg1.y = 0;

                if (exp_min_y > NEG_MAX_EXP) {
                    pin->e2.y = fastExp(exp_min_y);
                    sum_num2_bot.y += fp.y * pin->e2.y;
                    sum_denom2_bot.y += pin->e2.y;
                    pin->flg2.y = 1;
                }
                else pin->flg2.y = 0;
            }
        }
        if (curNet->cut == true && wlenIncludeTerm == true)
        { // Include terminal to calculate wirelength gradient
        // Considering the center of terminal as a pin, calculation becomes much easier. 
            terminal_ptr pin = (terminal_ptr)curNet->terminal;
            FPOS fp;
            double exp_max_x, exp_min_x, exp_max_y, exp_min_y;
            fp.x = pin->center.x;
            fp.y = pin->center.y;
            exp_max_x = (fp.x - curNet->pmax_bot.x) * wlen_cof_x;
            exp_min_x = (curNet->pmin_bot.x - fp.x) * wlen_cof_x;
            exp_max_y = (fp.y - curNet->pmax_bot.y) * wlen_cof_y;
            exp_min_y = (curNet->pmin_bot.y - fp.y) * wlen_cof_y;

            if (exp_max_x > NEG_MAX_EXP)
            {
                pin->e1.x = fastExp(exp_max_x);
                sum_num1_bot.x += fp.x * pin->e1.x;
                sum_denom1_bot.x += pin->e1.x;
                pin->flg1.x = 1;
            }
            else {
                pin->flg1.x = 0;
            }
            if (exp_min_x > NEG_MAX_EXP) {
                pin->e2.x = fastExp(exp_min_x);
                sum_num2_bot.x += fp.x * pin->e2.x;
                sum_denom2_bot.x += pin->e2.x;
                pin->flg2.x = 1;
            }
            else pin->flg2.x = 0;

            if (exp_max_y > NEG_MAX_EXP) {
                pin->e1.y = fastExp(exp_max_y);
                sum_num1_bot.y += fp.y * pin->e1.y;
                sum_denom1_bot.y += pin->e1.y;
                pin->flg1.y = 1;
            }
            else pin->flg1.y = 0;

            if (exp_min_y > NEG_MAX_EXP) {
                pin->e2.y = fastExp(exp_min_y);
                sum_num2_bot.y += fp.y * pin->e2.y;
                sum_denom2_bot.y += pin->e2.y;
                pin->flg2.y = 1;
            }
            else pin->flg2.y = 0;
        }
        if (sum_denom1_top.x == 0) sum_denom1_top.x = 1;
        if (sum_denom1_bot.x == 0) sum_denom1_bot.x = 1;
        if (sum_denom2_top.x == 0) sum_denom2_top.x = 1;
        if (sum_denom2_bot.x == 0) sum_denom2_bot.x = 1;
        if (sum_denom1_top.y == 0) sum_denom1_top.y = 1;
        if (sum_denom1_bot.y == 0) sum_denom1_bot.y = 1;
        if (sum_denom2_top.y == 0) sum_denom2_top.y = 1;
        if (sum_denom2_bot.y == 0) sum_denom2_bot.y = 1;
        curNet->sum_num1_top = sum_num1_top;
        curNet->sum_num2_top = sum_num2_top;
        curNet->sum_denom1_top = sum_denom1_top;
        curNet->sum_denom2_top = sum_denom2_top;

        curNet->sum_num1_bot = sum_num1_bot;
        curNet->sum_num2_bot = sum_num2_bot;
        curNet->sum_denom1_bot = sum_denom1_bot;
        curNet->sum_denom2_bot = sum_denom2_bot;
    }
}

struct FPOS get_instance_net_wlen_grad_wa(struct FCOORD obj, net_ptr curNet, net2inst_ptr pin)
{
    struct FPOS grad;
    grad.x = grad.y = 0.0;

    struct FPOS grad_sum_num1, grad_sum_num2;
    struct FPOS grad_sum_denom1, grad_sum_denom2;
    struct FPOS grad1;
    struct FPOS grad2;
    
    struct FPOS e1 = pin->e1;
    struct FPOS e2 = pin->e2;
    struct POS flg1 = pin->flg1;
    struct POS flg2 = pin->flg2;
    if (curNet->cut == false) // If current net is not cut
    {
        struct FPOS sum_num1;
        struct FPOS sum_num2;
        struct FPOS sum_denom1;
        struct FPOS sum_denom2;

        if (pin->instance->dieNum == 0) 
        {
            // This means that all instances in current net are in top die. 
            sum_num1 = curNet->sum_num1_top;
            sum_num2 = curNet->sum_num2_top;
            sum_denom1 = curNet->sum_denom1_top;
            sum_denom2 = curNet->sum_denom2_top;
        }
        else
        {
            // This means that all instances in current net are in top die
            sum_num1 = curNet->sum_num1_bot;
            sum_num2 = curNet->sum_num2_bot;
            sum_denom1 = curNet->sum_denom1_bot;
            sum_denom2 = curNet->sum_denom2_bot;
        }
        if(flg1.x) {
            grad_sum_denom1.x = wlen_cof_x * e1.x;
            grad_sum_num1.x = e1.x + obj.x * grad_sum_denom1.x;
            grad1.x =
                (grad_sum_num1.x * sum_denom1.x - grad_sum_denom1.x * sum_num1.x) /
                (sum_denom1.x * sum_denom1.x);
        }
        else grad1.x = 0.0;

        if(flg1.y) {
            grad_sum_denom1.y = wlen_cof_y * e1.y;
            grad_sum_num1.y = e1.y + obj.y * grad_sum_denom1.y;
            grad1.y =
                (grad_sum_num1.y * sum_denom1.y - grad_sum_denom1.y * sum_num1.y) /
                (sum_denom1.y * sum_denom1.y);
        }
        else grad1.y = 0.0;

        if(flg2.x) {
            grad_sum_denom2.x = wlen_cof_x * e2.x;
            grad_sum_num2.x = e2.x - obj.x * grad_sum_denom2.x;
            grad2.x =
                (grad_sum_num2.x * sum_denom2.x + grad_sum_denom2.x * sum_num2.x) /
                (sum_denom2.x * sum_denom2.x);
        }
        else grad2.x = 0.0;

        if(flg2.y) {
            grad_sum_denom2.y = wlen_cof_y * e2.y;
            grad_sum_num2.y = e2.y - obj.y * grad_sum_denom2.y;
            grad2.y =
                (grad_sum_num2.y * sum_denom2.y + grad_sum_denom2.y * sum_num2.y) /
                (sum_denom2.y * sum_denom2.y);
        }
        else grad2.y = 0.0;

        grad.x = grad1.x - grad2.x;
        grad.y = grad1.y - grad2.y;
        return grad;
    }
    else    // If current net is cut
    {
        struct FPOS sum_num1_top;
        struct FPOS sum_num1_bot;
        struct FPOS sum_num2_top;
        struct FPOS sum_num2_bot;

        struct FPOS sum_denom1_top;
        struct FPOS sum_denom1_bot;
        struct FPOS sum_denom2_top;
        struct FPOS sum_denom2_bot;
        double num_top, num_bot, denom_top, denom_bot;
        sum_num1_top = curNet->sum_num1_top;
        sum_num2_top = curNet->sum_num2_top;
        sum_denom1_top = curNet->sum_denom1_top;
        sum_denom2_top = curNet->sum_denom2_top;
        sum_num1_bot = curNet->sum_num1_bot;
        sum_num2_bot = curNet->sum_num2_bot;
        sum_denom1_bot = curNet->sum_denom1_bot;
        sum_denom2_bot = curNet->sum_denom2_bot;
        if(flg1.x) {
            grad_sum_denom1.x = wlen_cof_x * e1.x;
            grad_sum_num1.x = e1.x + obj.x * grad_sum_denom1.x;
            num_top = grad_sum_num1.x * sum_denom1_top.x - grad_sum_denom1.x * sum_num1_top.x;
            num_bot = grad_sum_num1.x * sum_denom1_bot.x - grad_sum_denom1.x * sum_num1_bot.x;
            denom_top = sum_denom1_top.x * sum_denom1_top.x;
            denom_bot = sum_denom1_bot.x * sum_denom1_bot.x;
            grad1.x = (num_top / denom_top) + (num_bot / denom_bot);
        }
        else grad1.x = 0.0;
        
        if(flg1.y) {
            grad_sum_denom1.y = wlen_cof_y * e1.y;
            grad_sum_num1.y = e1.y + obj.y * grad_sum_denom1.y;
            num_top = grad_sum_num1.y * sum_denom1_top.y - grad_sum_denom1.y * sum_num1_top.y;
            num_bot = grad_sum_num1.y * sum_denom1_bot.y - grad_sum_denom1.y * sum_num1_bot.y;
            denom_top = sum_denom1_top.y * sum_denom1_top.y;
            denom_bot = sum_denom1_bot.y * sum_denom1_bot.y;
            grad1.y = (num_top / denom_top) + (num_bot / denom_bot);
        }
        else grad1.y = 0.0;

        if(flg2.x) {
            grad_sum_denom2.x = wlen_cof_x * e2.x;
            grad_sum_num2.x = e2.x - obj.x * grad_sum_denom2.x;
            num_top = grad_sum_num2.x * sum_denom2_top.x - grad_sum_denom2.x * sum_num2_top.x;
            num_bot = grad_sum_num2.x * sum_denom2_bot.x - grad_sum_denom2.x * sum_num2_bot.x;
            denom_top = sum_denom2_top.x * sum_denom2_top.x;
            denom_bot = sum_denom2_bot.x * sum_denom2_bot.x;
            grad2.x = (num_top / denom_top) + (num_bot / denom_bot);
        }
        else grad2.x = 0.0;

        if(flg2.y) {
            grad_sum_denom2.y = wlen_cof_y * e2.y;
            grad_sum_num2.y = e2.y - obj.y * grad_sum_denom2.y;
            num_top = grad_sum_num2.y * sum_denom2_top.y - grad_sum_denom2.y * sum_num2_top.y;
            num_bot = grad_sum_num2.y * sum_denom2_bot.y - grad_sum_denom2.y * sum_num2_bot.y;
            denom_top = sum_denom2_top.y * sum_denom2_top.y;
            denom_bot = sum_denom2_bot.y * sum_denom2_bot.y;
            grad2.y = (num_top / denom_top) + (num_bot / denom_bot);
        }
        else grad2.y = 0.0;

        grad.x = grad1.x - grad2.x;
        grad.y = grad1.y - grad2.y;
        return grad;
    }
}


struct FPOS get_terminal_net_wlen_grad_wa(struct FCOORD obj, net_ptr curNet, terminal_ptr pin)
{
    struct FPOS grad;
    grad.x = grad.y = 0.0;

    struct FPOS grad_sum_num1, grad_sum_num2;
    struct FPOS grad_sum_denom1, grad_sum_denom2;
    struct FPOS grad1;
    struct FPOS grad2;
    
    struct FPOS e1 = pin->e1;
    struct FPOS e2 = pin->e2;
    struct POS flg1 = pin->flg1;
    struct POS flg2 = pin->flg2;
    struct FPOS sum_num1_top;
    struct FPOS sum_num1_bot;
    struct FPOS sum_num2_top;
    struct FPOS sum_num2_bot;

    struct FPOS sum_denom1_top;
    struct FPOS sum_denom1_bot;
    struct FPOS sum_denom2_top;
    struct FPOS sum_denom2_bot;
    double num_top, num_bot, denom_top, denom_bot;
    sum_num1_top = curNet->sum_num1_top;
    sum_num2_top = curNet->sum_num2_top;
    sum_denom1_top = curNet->sum_denom1_top;
    sum_denom2_top = curNet->sum_denom2_top;
    sum_num1_bot = curNet->sum_num1_bot;
    sum_num2_bot = curNet->sum_num2_bot;
    sum_denom1_bot = curNet->sum_denom1_bot;
    sum_denom2_bot = curNet->sum_denom2_bot;
    // if (!sum_denom1_bot.x || !sum_denom1_bot.y || !sum_denom1_top.x || !sum_denom1_top.y || !sum_denom2_bot.x || !sum_denom2_bot.y || !sum_denom2_top.x || !sum_denom2_top.y) printf("%3lf, %3lf, %3lf, %3lf, %3lf, %3lf, %3lf, %3lf\n", sum_denom1_top.x, sum_denom1_top.y, sum_denom1_bot.x, sum_denom1_bot.y, sum_denom2_top.x, sum_denom2_top.y, sum_denom2_bot.x, sum_denom2_bot.y);
    if(flg1.x) {
        grad_sum_denom1.x = wlen_cof_x * e1.x;
        grad_sum_num1.x = e1.x + obj.x * grad_sum_denom1.x;
        num_top = grad_sum_num1.x * sum_denom1_top.x - grad_sum_denom1.x * sum_num1_top.x;
        num_bot = grad_sum_num1.x * sum_denom1_bot.x - grad_sum_denom1.x * sum_num1_bot.x;
        denom_top = sum_denom1_top.x * sum_denom1_top.x;
        denom_bot = sum_denom1_bot.x * sum_denom1_bot.x;
        grad1.x = (num_top / denom_top) + (num_bot / denom_bot);
    }
    else grad1.x = 0.0;

    if(flg1.y) {
        grad_sum_denom1.y = wlen_cof_y * e1.y;
        grad_sum_num1.y = e1.y + obj.y * grad_sum_denom1.y;
        num_top = grad_sum_num1.y * sum_denom1_top.y - grad_sum_denom1.y * sum_num1_top.y;
        num_bot = grad_sum_num1.y * sum_denom1_bot.y - grad_sum_denom1.y * sum_num1_bot.y;
        denom_top = sum_denom1_top.y * sum_denom1_top.y;
        denom_bot = sum_denom1_bot.y * sum_denom1_bot.y;
        grad1.y = (num_top / denom_top) + (num_bot / denom_bot);
    }
    else grad1.y = 0.0;

    if(flg2.x) {
        grad_sum_denom2.x = wlen_cof_x * e2.x;
        grad_sum_num2.x = e2.x - obj.x * grad_sum_denom2.x;
        num_top = grad_sum_num2.x * sum_denom2_top.x - grad_sum_denom2.x * sum_num2_top.x;
        num_bot = grad_sum_num2.x * sum_denom2_bot.x - grad_sum_denom2.x * sum_num2_bot.x;
        denom_top = sum_denom2_top.x * sum_denom2_top.x;
        denom_bot = sum_denom2_bot.x * sum_denom2_bot.x;
        grad2.x = (num_top / denom_top) + (num_bot / denom_bot);
    }
    else grad2.x = 0.0;

    if(flg2.y) {
        grad_sum_denom2.y = wlen_cof_y * e2.y;
        grad_sum_num2.y = e2.y - obj.y * grad_sum_denom2.y;
        num_top = grad_sum_num2.y * sum_denom2_top.y - grad_sum_denom2.y * sum_num2_top.y;
        num_bot = grad_sum_num2.y * sum_denom2_bot.y - grad_sum_denom2.y * sum_num2_bot.y;
        denom_top = sum_denom2_top.y * sum_denom2_top.y;
        denom_bot = sum_denom2_bot.y * sum_denom2_bot.y;
        grad2.y = (num_top / denom_top) + (num_bot / denom_bot);
    }
    else grad2.y = 0.0;

    grad.x = grad1.x - grad2.x;
    grad.y = grad1.y - grad2.y;
    return grad;
}


void instance_wlen_grad_wa(instance_ptr curInst)
{
    inst2net_ptr pin = NULL;
    net2inst_ptr net_pin = NULL;
    net_ptr curNet = NULL;
    struct FPOS net_grad;
    curInst->wlen_grad.x = curInst->wlen_grad.y = 0.0;

    pin = curInst->net_head;


    int topInst, botInst;
    topInst = botInst = 0;
    while(pin)
    {
        net_pin = (net2inst_ptr)pin->to_netLink;
        curNet = (net_ptr)pin->net;
        net_grad = get_instance_net_wlen_grad_wa(pin->curCoord, curNet, net_pin);
        curInst->wlen_grad.x += net_grad.x;
        curInst->wlen_grad.y += net_grad.y;
        // printf("Inst %s Pin %s grad (%6lf, %6lf)\n", curInst->instanceName, pin->pinName, net_grad.x, net_grad.y);
        // net2inst_ptr netLink = curNet->instance_head;
        // while(netLink)
        // {
        //     if (netLink->instance->dieNum == 0) topInst++;
        //     else botInst++;
        //     netLink = netLink->next;
        // }
        // printf("Top inst num = %d, bot Inst num = %d\n", topInst, botInst);
        pin = pin->next;
    }
}


void terminal_wlen_grad_wa(terminal_ptr curTerm)
{
    net_ptr curNet = curTerm->term_net;
    curTerm->wlen_grad = get_terminal_net_wlen_grad_wa(curTerm->center, curNet, curTerm);
}


void update_global_wlen_grad_wa(dataBase_ptr data)
{
    #pragma omp parallel default(none) shared(data)
    #pragma omp for
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = data->instanceDB->inst_array[i];
        instance_wlen_grad_wa(curInst);
    }
    #if wlenIncludeTerm
    #pragma omp parallel default(none) shared(data)
    #pragma omp for
    for (int i = 0; i < data->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = data->terminalDB->term_array[i];
        terminal_wlen_grad_wa(curTerm);
    }
    #endif
}