#include "opt.h"

using namespace std::chrono;

void opt_init(dataBase_ptr data, fftDB_ptr fftDB, binDB_ptr binDB, struct POS binSize)
{
    system_clock::time_point start = system_clock::now();
    #ifdef OPT_VERBOSE
    printf("###### OPT init #######\n");
    #endif
    fftDB = create_FFTDB(data);
    binDB = createBin(data, binSize);
    net_update_wa(data);
    system_clock::time_point end = system_clock::now();
    #ifdef OPT_VERBOSE
    milliseconds runtime = duration_cast<milliseconds>(end - start);
    std::cout << "OPT Init time: "  << runtime.count() << "ms\n";
    #endif
}


void calc_gradient(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB)
{
    system_clock::time_point start = system_clock::now();
    #ifdef OPT_VERBOSE
    printf("###### Gradient Calculation #######\n");
    #endif
    // Charge Gradient
    generate_global_density(data, fftDB_top, fftDB_bot, fftDB_term);
    #ifdef OPT_VERBOSE
    system_clock::time_point grad_gen = system_clock::now();
    milliseconds grad_gentime = duration_cast<milliseconds>(grad_gen - start);
    std::cout << "grad_gen time: "  << grad_gentime.count() << "ms\n";
    #endif
    charge_fft_call_2d(fftDB_top);
    charge_fft_call_2d(fftDB_bot);
    charge_fft_call_2d(fftDB_term);
    #ifdef OPT_VERBOSE
    system_clock::time_point fft = system_clock::now();
    milliseconds fft_time = duration_cast<milliseconds>(fft - grad_gen);
    std::cout << "fft time: "  << fft_time.count() << "ms\n";
    #endif
    potn_grad_2d_global(data, fftDB_top, fftDB_bot, binDB);
    potn_grad_2d_global_term(data, fftDB_term, binDB);
    #ifdef OPT_VERBOSE
    system_clock::time_point potn = system_clock::now();
    milliseconds potn_time = duration_cast<milliseconds>(potn - fft);
    std::cout << "potn time: "  << fft_time.count() << "ms\n";
    #endif

    // Wlen gradient
    update_global_wlen_grad_wa(data);
    system_clock::time_point end = system_clock::now();
    #ifdef OPT_VERBOSE
    milliseconds runtime = duration_cast<milliseconds>(end - start);
    std::cout << "Gradient Calculation time: "  << runtime.count() << "ms\n";
    #endif
}


void move_instance_local(dataBase_ptr data, instance_ptr curInst, double lr, struct FPOS momentum, double charge_weight)
{
    struct FPOS grad, pmin, pmax;
    curInst->grad.x = curInst->charge_grad.x * charge_weight + curInst->wlen_grad.x;
    curInst->grad.y = curInst->charge_grad.y * charge_weight + curInst->wlen_grad.y;
    curInst->center.x += curInst->grad.x * lr * momentum.x;
    curInst->center.y += curInst->grad.y + lr * momentum.y;
    pmin.x = curInst->center.x - (double)curInst->sizeX * 0.5;
    pmin.y = curInst->center.y - (double)curInst->sizeY * 0.5;
    pmax.x = curInst->center.x + (double)curInst->sizeX * 0.5;
    pmax.y = curInst->center.y + (double)curInst->sizeY * 0.5;
    if (pmin.x < data->dieDB->lowerLeftX) curInst->center.x = (double)data->dieDB->lowerLeftX + (double)curInst->sizeX * 0.5 + GP_epsilon;
    if (pmin.y < data->dieDB->lowerLeftY) curInst->center.y = (double)data->dieDB->lowerLeftY + (double)curInst->sizeY * 0.5 + GP_epsilon;
    if (pmax.x > data->dieDB->upperRightX) curInst->center.x = (double)data->dieDB->upperRightX - (double)curInst->sizeX * 0.5 - GP_epsilon;
    if (pmax.y > data->dieDB->upperRightY) curInst->center.y = (double)data->dieDB->upperRightY - (double)curInst->sizeY * 0.5 - GP_epsilon;
}


void move_instance_global(dataBase_ptr data, double lr, struct FPOS momentum, double charge_weight)
{
    system_clock::time_point start = system_clock::now();
    #ifdef OPT_VERBOSE
    printf("###### Analytic Placement #######\n");
    #endif
    #pragma omp parallel default(none) shared(data, lr, momentum, charge_weight)
    #pragma omp for
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = data->instanceDB->inst_array[i];
        move_instance_local(data, curInst, lr, momentum, charge_weight);
    }
    system_clock::time_point end = system_clock::now();
    #ifdef OPT_VERBOSE
    milliseconds runtime = duration_cast<milliseconds>(end - start);
    std::cout << "Analytic Placement time: "  << runtime.count() << "ms\n";
    #endif
}


void move_terminal_local(dataBase_ptr data, terminal_ptr curTerm, double lr, struct FPOS momentum, double charge_weight)
{
    struct FPOS grad, pmin, pmax, expanded_size;
    curTerm->grad.x = curTerm->charge_grad.x * charge_weight + curTerm->wlen_grad.x;
    curTerm->grad.y = curTerm->charge_grad.y * charge_weight + curTerm->wlen_grad.y;
    curTerm->center.x += curTerm->grad.x * lr * momentum.x;
    curTerm->center.y += curTerm->grad.y * lr * momentum.y;
    // For the cases it reaches the end
    expanded_size.x = (double)curTerm->sizeX + (double)curTerm->spacing;
    expanded_size.y = (double)curTerm->sizeY + (double)curTerm->spacing;
    pmin.x = curTerm->center.x - expanded_size.x * 0.5;
    pmin.y = curTerm->center.y - expanded_size.y * 0.5;
    pmax.x = curTerm->center.x + expanded_size.x * 0.5;
    pmax.y = curTerm->center.y + expanded_size.y * 0.5;
    if (pmin.x < data->dieDB->lowerLeftX) curTerm->center.x = (double)data->dieDB->lowerLeftX + expanded_size.x * 0.5 + GP_epsilon;
    if (pmin.y < data->dieDB->lowerLeftY) curTerm->center.y = (double)data->dieDB->lowerLeftY + expanded_size.y * 0.5 + GP_epsilon;
    if (pmax.x > data->dieDB->upperRightX) curTerm->center.x = (double)data->dieDB->upperRightX - expanded_size.x * 0.5 - GP_epsilon;
    if (pmax.y > data->dieDB->upperRightY) curTerm->center.y = (double)data->dieDB->upperRightY - expanded_size.y * 0.5 - GP_epsilon;
}


void move_terminal_global(dataBase_ptr data, double lr, struct FPOS momentum, double charge_weight)
{
    #pragma omp parallel default(none) shared(data, lr, momentum, charge_weight)
    #pragma omp for
    for (int i = 0; i < data->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = data->terminalDB->term_array[i];
        move_terminal_local(data, curTerm, lr, momentum, charge_weight);
    }
}


void post_movement_func(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term, binDB_ptr binDB)
{
    #ifdef OPT_VERBOSE
    system_clock::time_point start = system_clock::now();
    printf("###### Post movement #######\n");
    #endif

    update_pin_by_inst(data);
    update_net_pos(data);
    net_update_wa(data);
    updateBinDB(data, binDB);
    reset_global_density(fftDB_top);
    reset_global_density(fftDB_bot);
    reset_global_density(fftDB_term);
    #ifdef OPT_VERBOSE
    system_clock::time_point end = system_clock::now();
    milliseconds runtime = duration_cast<milliseconds>(end - start);
    std::cout << "Post movement time: "  << runtime.count() << "ms\n";
    #endif
}