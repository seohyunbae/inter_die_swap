#include "charge.h"


void generate_global_density(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, fftDB_ptr fftDB_term)
{
    instance_ptr curInst;
    // Multithreading prohibitted!!!!!
    // Touching fftDB at the same time can cause a race condition
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];
        if (curInst->dieNum == 0) generate_instance_density(curInst, fftDB_top);
        else generate_instance_density(curInst, fftDB_bot);
    }
    if (chargeIncludeTerm)
    {
        for (int i = 0; i < data->terminalDB->numTerminals; i++)
        {
            terminal_ptr curTerm = data->terminalDB->term_array[i];
            generate_terminal_density(curTerm, fftDB_term);
        }
    }
}


void generate_instance_density(instance_ptr curInst, fftDB_ptr fftDB)
{
    using namespace std;
    struct FPOS fLowerLeft, fUpperRight, resLowerLeft, resUpperRight;
    struct POS lowerLeft, upperRight;
    // Get FFT bin coord
    fLowerLeft.x = curInst->center.x - ( (double)curInst->sizeX / 2.0 );
    fLowerLeft.y = curInst->center.y - ( (double)curInst->sizeY / 2.0 );
    fUpperRight.x = curInst->center.x + ( (double)curInst->sizeX / 2.0);
    fUpperRight.y = curInst->center.y + ( (double)curInst->sizeY / 2.0);
    lowerLeft.x = (int)(fLowerLeft.x / fftDB->dft_bin_size.x);
    lowerLeft.y = (int)(fLowerLeft.y / fftDB->dft_bin_size.y);
    upperRight.x = (int)(fUpperRight.x / fftDB->dft_bin_size.x);
    upperRight.y = (int)(fUpperRight.y / fftDB->dft_bin_size.y);
    curInst->fft_coord_min = lowerLeft;
    curInst->fft_coord_max = upperRight;

    // Residual Calculation
    resLowerLeft.x = fftDB->dft_bin_size.x * (lowerLeft.x + 1) - fLowerLeft.x;
    resLowerLeft.y = fftDB->dft_bin_size.y * (lowerLeft.y + 1) - fLowerLeft.y;
    resUpperRight.x = fUpperRight.x - fftDB->dft_bin_size.x * (upperRight.x);
    resUpperRight.y = fUpperRight.y - fftDB->dft_bin_size.y * (upperRight.y);
    for (int x = lowerLeft.x; x < upperRight.x; x++)
    {
        for (int y = lowerLeft.y; y < upperRight.y; y++)
        {
            if (x == 0)
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += fp_product(resLowerLeft);
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += resLowerLeft.x * resUpperRight.y;
                else fftDB->den_2d_st2[x][y] += resLowerLeft.x * fftDB->dft_bin_size.y;
            }
            else if (x == upperRight.x)
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += resUpperRight.x * resLowerLeft.y;
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += fp_product(resUpperRight);
                else fftDB->den_2d_st2[x][y] += resUpperRight.x * fftDB->dft_bin_size.y;
            }
            else
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += fftDB->dft_bin_size.x * resLowerLeft.y;
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += fftDB->dft_bin_size.x * resUpperRight.y;
                else fftDB->den_2d_st2[x][y] += fp_product(fftDB->dft_bin_size);
            }
        }
    }
}


void generate_terminal_density(terminal_ptr curTerm, fftDB_ptr fftDB)
{
    struct FPOS fLowerLeft, fUpperRight, resLowerLeft, resUpperRight;
    struct POS lowerLeft, upperRight;
    // Get FFT bin coord
    
    fLowerLeft.x = curTerm->center.x - ((double)curTerm->sizeX + (double)curTerm->spacing) / 2.0;
    fLowerLeft.y = curTerm->center.y - ((double)curTerm->sizeY + (double)curTerm->spacing) / 2.0;
    fUpperRight.x = curTerm->center.x + ((double)curTerm->sizeX + (double)curTerm->spacing) / 2.0;
    fUpperRight.y = curTerm->center.y + ((double)curTerm->sizeY + (double)curTerm->spacing) / 2.0;
    lowerLeft.x = (int)(fLowerLeft.x / fftDB->dft_bin_size.x);
    lowerLeft.y = (int)(fLowerLeft.y / fftDB->dft_bin_size.y);
    upperRight.x = (int)(fUpperRight.x / fftDB->dft_bin_size.x);
    upperRight.y = (int)(fUpperRight.y / fftDB->dft_bin_size.y);

    curTerm->fft_coord_min = lowerLeft;
    curTerm->fft_coord_max = upperRight;
    // printf("Term size = %d %d %d\n", curTerm->sizeX, curTerm->sizeY, curTerm->spacing);
    // printf("@ (%3lf, %3lf) to (%3lf, %3lf)\n", pmin.x, pmin.y, pmax.x, pmax.y);
    // printf("%d -> %d , %d -> %d\n", curTerm->fft_coord_min.x, curTerm->fft_coord_max.x, curTerm->fft_coord_min.y, curTerm->fft_coord_max.y);

    // Residual Calculation
    resLowerLeft.x = fftDB->dft_bin_size.x * (lowerLeft.x + 1) - fLowerLeft.x;
    resLowerLeft.y = fftDB->dft_bin_size.y * (lowerLeft.y + 1) - fLowerLeft.y;
    resUpperRight.x = fUpperRight.x - fftDB->dft_bin_size.x * (upperRight.x);
    resUpperRight.y = fUpperRight.y - fftDB->dft_bin_size.y * (upperRight.y);

    // Density calculated with overlapping area with bin
    for (int x = lowerLeft.x; x < upperRight.x; x++)
    {
        for (int y = lowerLeft.y; y < lowerLeft.y; y++)
        {
            if (x == 0)
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += fp_product(resLowerLeft);
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += resLowerLeft.x * resUpperRight.y;
                else fftDB->den_2d_st2[x][y] += resLowerLeft.x * fftDB->dft_bin_size.y;
            }
            else if (x == upperRight.x)
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += resUpperRight.x * resLowerLeft.y;
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += fp_product(resUpperRight);
                else fftDB->den_2d_st2[x][y] += resUpperRight.x * fftDB->dft_bin_size.y;
            }
            else
            {
                if (y == 0) fftDB->den_2d_st2[x][y] += fftDB->dft_bin_size.x * resLowerLeft.y;
                else if (y == upperRight.y) fftDB->den_2d_st2[x][y] += fftDB->dft_bin_size.x * resUpperRight.y;
                else fftDB->den_2d_st2[x][y] += fp_product(fftDB->dft_bin_size);
            }
        }
    }
}


void reset_global_density(fftDB_ptr fftDB)
{
    for (int x = 0; x < fftDB->dft_bin_2d.x; x++)
    {
        for (int y = 0; y < fftDB->dft_bin_2d.y; y++)
        {
            fftDB->den_2d_st2[x][y] = 0;
        }
    }
}


void potn_grad_2d_global(dataBase_ptr data, fftDB_ptr fftDB_top, fftDB_ptr fftDB_bot, binDB_ptr binDB)
{
    omp_set_num_threads(NumThreads);
    #pragma omp parallel default(none) shared(data, fftDB_top, fftDB_bot, binDB)
    #pragma omp for
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr curInst = data->instanceDB->inst_array[i];
        if (curInst->dieNum == 0) potn_grad_2D_local(curInst, NULL, fftDB_top, binDB);
        else potn_grad_2D_local(curInst, NULL, fftDB_bot, binDB);
    }
}

void potn_grad_2d_global_term(dataBase_ptr data, fftDB_ptr fftDB_term, binDB_ptr binDB)
{
    omp_set_num_threads(NumThreads);
    #pragma omp parallel default(none) shared(data, fftDB_term, binDB)
    #pragma omp for
    for (int i = 0; i < data->terminalDB->numTerminals; i++)
    {
        terminal_ptr curTerm = data->terminalDB->term_array[i];
        potn_grad_2d_local_term(curTerm, NULL, fftDB_term, binDB);
    }
}


void potn_grad_2D_local(instance_ptr curInst, double* cellLambda, fftDB_ptr fftDB, binDB_ptr binDB)
{
    using namespace std;
    double area_share = 0.0;
    double bpx_min_x = 0.0;
    double bpx_min_y = 0.0;
    double bpx_max_x = 0.0;
    double bpx_max_y = 0.0;
    double bpy_min_x = 0.0;
    double bpy_min_y = 0.0;
    double bpy_max_x = 0.0;
    double bpy_max_y = 0.0;
    double bpx_heightY;
    double bpx_delta_x_movement = 0.0;
    double bpy_heightX;
    double bpy_delta_y_movement = 0.0;
    double exp_term = 0.0;
    double common_val = 0.0;
    double common_div = 0.0;
    double common_mul = 0.0;

    struct POS b0, b1;
    curInst->charge_grad.x = curInst->charge_grad.y = 0.0;
    struct FPOS instLowerLeft, instUpperRight, fftLowerLeft, fftUpperRight;
    struct FBBOX fftbbox;
    struct BIN curBin = (curInst->dieNum == 0) ? binDB->binMat2d_top[curInst->binCoord.x][curInst->binCoord.y]:binDB->binMat2d_bot[curInst->binCoord.x][curInst->binCoord.y];
    double alpha = ALPHA;

    instLowerLeft.x = curInst->center.x - (double)curInst->sizeX / 2.0;
    instLowerLeft.y = curInst->center.y - (double)curInst->sizeY / 2.0;
    instUpperRight.x = curInst->center.x + (double)curInst->sizeX / 2.0;
    instUpperRight.y = curInst->center.y + (double)curInst->sizeY / 2.0;


    // Calculation of penalty factor. 
    // Start calculation with bin DB. 
    common_div = alpha / (binDB->binStep.x * binDB->binStep.y);
    double share_x = (min(curBin.upperRight.x, instUpperRight.x) - max(curBin.lowerLeft.x, instLowerLeft.x));
    double share_y = (min(curBin.upperRight.y, instUpperRight.y) - max(curBin.lowerLeft.y, instLowerLeft.y));
    area_share = share_x * share_y;
    exp_term = fastExp(common_div * (curBin.instArea - (binDB->binStep.x * binDB->binStep.y)));

    fftLowerLeft.x = curInst->fft_coord_min.x * fftDB->dft_bin_size.x;
    fftLowerLeft.y = curInst->fft_coord_min.y * fftDB->dft_bin_size.y;
    fftUpperRight.x = (curInst->fft_coord_max.x + 1) * fftDB->dft_bin_size.x;
    fftUpperRight.y = (curInst->fft_coord_max.y + 1) * fftDB->dft_bin_size.y;


    for (int x = curInst->fft_coord_min.x; x < curInst->fft_coord_max.x; x++)
    {
        fftbbox.pmin.x = fftDB->dft_bin_size.x * x;
        fftbbox.pmin.y = fftDB->dft_bin_size.y * curInst->fft_coord_min.y;
        fftbbox.pmax.x = fftDB->dft_bin_size.x + fftbbox.pmin.x;
        fftbbox.pmax.y = fftDB->dft_bin_size.y + fftbbox.pmin.y;

        bpx_max_x = min(fftbbox.pmax.x, instUpperRight.x);
        bpx_min_x = max(fftbbox.pmin.x, instLowerLeft.x);
        bpx_max_y = min(fftbbox.pmax.y, instUpperRight.y);
        bpx_min_y = max(fftbbox.pmin.y, instUpperRight.y);

        bpx_heightY = bpx_max_y - bpx_min_y;
        bpx_delta_x_movement = fabs(bpx_heightY);
        if (bpx_max_x == fftbbox.pmax.x && bpx_min_x == fftbbox.pmin.x) bpx_delta_x_movement = 0;
        else if (bpx_max_x == fftbbox.pmax.x) bpx_delta_x_movement *= -1;

        for (int y = curInst->fft_coord_min.y; y < curInst->fft_coord_max.y; y++)
        {
            fftbbox.pmin.y = fftDB->dft_bin_size.y * y;
            fftbbox.pmax.y = fftDB->dft_bin_size.y + fftbbox.pmin.y;
            bpy_max_x = bpx_max_x;
            bpy_min_x = bpx_min_x;
            bpy_max_y = min(fftbbox.pmax.y, instUpperRight.y);
            bpy_min_y = max(fftbbox.pmin.y, instLowerLeft.y);

            bpy_heightX = bpy_max_x - bpy_min_y;
            bpy_delta_y_movement = fabs(bpy_heightX);
            if (bpy_max_y == fftbbox.pmax.y && bpy_min_y == fftbbox.pmin.y) bpy_delta_y_movement = 0;
            else if (bpy_max_y == fftbbox.pmax.y) bpy_delta_y_movement *= -1;

            common_val = common_div * fftDB->phi_2d_st2[x][y] * exp_term * curBin.instArea;
            common_mul = exp_term * area_share;
            
            curInst->charge_grad.x += bpx_delta_x_movement * common_val;
            curInst->charge_grad.x += common_mul * fftDB->ex_2d_st2[x][y];
            
            curInst->charge_grad.y += bpy_delta_y_movement * common_val;
            curInst->charge_grad.y += common_mul * fftDB->ey_2d_st2[x][y];
        }
    }
}


void potn_grad_2d_local_term(terminal_ptr curTerm, double* cellLambda, fftDB_ptr fftDB, binDB_ptr binDB)
{
    using namespace std;
    double area_share = 0.0;
    double bpx_min_x = 0.0;
    double bpx_min_y = 0.0;
    double bpx_max_x = 0.0;
    double bpx_max_y = 0.0;
    double bpy_min_x = 0.0;
    double bpy_min_y = 0.0;
    double bpy_max_x = 0.0;
    double bpy_max_y = 0.0;
    double bpx_heightY;
    double bpx_delta_x_movement = 0.0;
    double bpy_heightX;
    double bpy_delta_y_movement = 0.0;
    double exp_term = 0.0;
    double common_val = 0.0;
    double common_div = 0.0;
    double common_mul = 0.0;

    struct POS b0, b1;
    curTerm->charge_grad.x = curTerm->charge_grad.y = 0.0;
    struct FPOS pmin, pmax, fftpmin, fftpmax;
    struct FBBOX fftbbox;
    struct BIN curBin = binDB->binMat2d_term[curTerm->binCoord.x][curTerm->binCoord.y];
    double alpha = ALPHA;

    pmin.x = curTerm->center.x - ((double)curTerm->sizeX + (double)curTerm->spacing) / 2.0;
    pmin.y = curTerm->center.y - ((double)curTerm->sizeY + (double)curTerm->spacing) / 2.0;
    pmax.x = curTerm->center.x + ((double)curTerm->sizeX + (double)curTerm->spacing) / 2.0;
    pmax.y = curTerm->center.y + ((double)curTerm->sizeY + (double)curTerm->spacing) / 2.0;

    common_div = alpha / (binDB->binStep.x * binDB->binStep.y);
    double share_x = (min(curBin.upperRight.x, pmax.x) - max(curBin.lowerLeft.x, pmin.x));
    double share_y = (min(curBin.upperRight.y, pmax.y) - max(curBin.lowerLeft.y, pmin.y));
    area_share = share_x * share_y;
    exp_term = fastExp(common_div * (curBin.termArea - (binDB->binStep.x * binDB->binStep.y)));

    fftpmin.x = curTerm->fft_coord_min.x * fftDB->dft_bin_size.x;
    fftpmin.y = curTerm->fft_coord_min.x * fftDB->dft_bin_size.y;
    fftpmax.x = (curTerm->fft_coord_max.x + 1) * fftDB->dft_bin_size.x;
    fftpmax.y = (curTerm->fft_coord_max.x + 1) * fftDB->dft_bin_size.y;


    for (int x = curTerm->fft_coord_min.x; x < curTerm->fft_coord_max.x; x++)
    {
        fftbbox.pmin.x = fftDB->dft_bin_size.x * x;
        fftbbox.pmin.y = fftDB->dft_bin_size.y * curTerm->fft_coord_min.y;
        fftbbox.pmax.x = fftDB->dft_bin_size.x + fftbbox.pmin.x;
        fftbbox.pmax.y = fftDB->dft_bin_size.y + fftbbox.pmin.y;

        bpx_max_x = min(fftbbox.pmax.x, pmax.x);
        bpx_min_x = max(fftbbox.pmin.x, pmin.x);
        bpx_max_y = min(fftbbox.pmax.y, pmax.y);
        bpx_min_y = max(fftbbox.pmin.y, pmin.y);

        bpx_heightY = bpx_max_y - bpx_min_y;
        bpx_delta_x_movement = fabs(bpx_heightY);
        if (bpx_max_x == fftbbox.pmax.x && bpx_min_x == fftbbox.pmin.x) bpx_delta_x_movement = 0;
        else if (bpx_max_x == fftbbox.pmax.x) bpx_delta_x_movement *= -1;

        for (int y = curTerm->fft_coord_min.y; y < curTerm->fft_coord_max.y; y++)
        {
            fftbbox.pmin.y = fftDB->dft_bin_size.y * y;
            fftbbox.pmax.y = fftDB->dft_bin_size.y + fftbbox.pmin.y;
            bpy_max_x = bpx_max_x;
            bpy_min_x = bpx_min_x;
            bpy_max_y = min(fftbbox.pmax.y, pmax.y);
            bpy_min_y = max(fftbbox.pmin.y, pmin.y);

            bpy_heightX = bpy_max_x - bpy_min_y;
            bpy_delta_y_movement = fabs(bpy_heightX);
            if (bpy_max_y == fftbbox.pmax.y && bpy_min_y == fftbbox.pmin.y) bpy_delta_y_movement = 0;
            else if (bpy_max_y == fftbbox.pmax.y) bpy_delta_y_movement *= -1;

            common_val = common_div * fftDB->phi_2d_st2[x][y] * exp_term * curBin.instArea;
            common_mul = exp_term * area_share;
            
            curTerm->charge_grad.x += bpx_delta_x_movement * common_val;
            curTerm->charge_grad.x += common_mul * fftDB->ex_2d_st2[x][y];
            
            curTerm->charge_grad.y += bpy_delta_y_movement * common_val;
            curTerm->charge_grad.y += common_mul * fftDB->ey_2d_st2[x][y];
        }
    }
}