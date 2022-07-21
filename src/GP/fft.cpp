#include "fft.h"


struct POS getFFTCoord(fftDB_ptr fftDB, instance_ptr inst)
{
    FCOORD center = inst->center;
    FPOS binStep = fftDB->dft_bin_size;
    POS fftCoord;
    fftCoord.x = (int)floor(center.x / binStep.x);
    fftCoord.y = (int)floor(center.y / binStep.y);
    return fftCoord;
}


fftDB_ptr create_FFTDB(dataBase_ptr data)
{
    fftDB_ptr fftDB = (fftDB_ptr)malloc(sizeof(struct FFT_DB));
    // Calculation of DFT bin step size and DFT bin step number
    struct POS dft_bin_2d;
    struct FPOS dft_bin_size, dieSize;
    double DFT_SCALE_2D;

    dieSize.x = (double)data->dieDB->upperRightX - (double)data->dieDB->lowerLeftX;
    dieSize.y = (double)data->dieDB->upperRightY - (double)data->dieDB->lowerLeftY;
    dft_bin_2d.x = calc_largest_po2((int)dieSize.x);
    dft_bin_2d.y = calc_largest_po2((int)dieSize.y);
    dft_bin_size.x = dieSize.x / dft_bin_2d.x;
    dft_bin_size.y = dieSize.y / dft_bin_2d.y;

    if (dftBinSizeX != 1)
    {
        dft_bin_2d.x /= dftBinScale;
        dft_bin_2d.y /= dftBinScale;
        dft_bin_size.x *= dftBinScale;
        dft_bin_size.y *= dftBinScale;
    }

    // fftDB->dft_bin_2d = dft_bin_2d;
    // fftDB->dft_bin_size = dft_bin_size;
    fftDB->dft_bin_2d.x = dft_bin_2d.x;
    fftDB->dft_bin_2d.y = dft_bin_2d.y;
    fftDB->dft_bin_size.x = dft_bin_size.x;
    fftDB->dft_bin_size.y = dft_bin_size.y;
    fftDB->charge_dft_n_2d = p_max(dft_bin_2d);
    fftDB->charge_dft_nbin_2d = p_product(dft_bin_2d);
    DFT_SCALE_2D = 1.0 / (double)(fftDB->charge_dft_n_2d);

    fftDB->den_2d_st2 = (double**)malloc(sizeof(double*) * dft_bin_2d.x);
    fftDB->phi_2d_st2 = (double**)malloc(sizeof(double*) * dft_bin_2d.x);
    fftDB->ex_2d_st2 = (double**)malloc(sizeof(double*) * dft_bin_2d.x);
    fftDB->ey_2d_st2 = (double**)malloc(sizeof(double*) * dft_bin_2d.x);
    for (int x = 0; x < dft_bin_2d.x; x++)
    {
        fftDB->den_2d_st2[x] = (double*)malloc(sizeof(double) * dft_bin_2d.y);
        fftDB->phi_2d_st2[x] = (double*)malloc(sizeof(double) * dft_bin_2d.y);
        fftDB->ex_2d_st2[x] = (double*)malloc(sizeof(double) * dft_bin_2d.y);
        fftDB->ey_2d_st2[x] = (double*)malloc(sizeof(double) * dft_bin_2d.y);
        for (int y = 0; y < dft_bin_2d.y; y++)
        {
            fftDB->den_2d_st2[x][y] = 0;
        }
    }
    fftDB->charge_dft_nbit_2d = 2 + (int)sqrt((double)(fftDB->charge_dft_n_2d + 0.5));
    fftDB->charge_ip_2d = (int *)malloc(sizeof(int) * fftDB->charge_dft_nbit_2d);
    fftDB->charge_dft_nw_2d = fftDB->charge_dft_n_2d * 3 / 2;
    fftDB->w_2d = (double*)malloc(sizeof(double) * fftDB->charge_dft_nw_2d);
    fftDB->charge_ip_2d[0] = 0;

    fftDB->wx_2d_st = (double*)malloc(sizeof(double) * dft_bin_2d.x);
    fftDB->wy_2d_st = (double*)malloc(sizeof(double) * dft_bin_2d.y);
    fftDB->wx2_2d_st = (double*)malloc(sizeof(double) * dft_bin_2d.x);
    fftDB->wy2_2d_st = (double*)malloc(sizeof(double) * dft_bin_2d.y);
    for (int x = 0; x < dft_bin_2d.x; x++)
    {
        fftDB->wx_2d_st[x] = PI * (double)x / ((double)dft_bin_2d.x);
        fftDB->wx2_2d_st[x] = (fftDB->wx_2d_st[x]) * (fftDB->wx_2d_st[x]);
    }
    for (int y = 0; y < dft_bin_2d.y; y++)
    {
        fftDB->wy_2d_st[y] = PI * (double)y / ((double)dft_bin_2d.y * fftDB->dft_bin_size.y / (fftDB->dft_bin_size.x));
        fftDB->wy2_2d_st[y] = (fftDB->wy_2d_st[y]) * (fftDB->wy_2d_st[y]);
    }
    return fftDB;
}


void destroy_FFTDB(fftDB_ptr rm_db)
{
    int X = rm_db->dft_bin_2d.x;
    int Y = rm_db->dft_bin_2d.y;
    for (int x = 0; x < X; x++)
    {
        if(rm_db->den_2d_st2[x]) free(rm_db->den_2d_st2[x]);
        if(rm_db->phi_2d_st2[x]) free(rm_db->phi_2d_st2[x]);
        if(rm_db->ex_2d_st2[x]) free(rm_db->ex_2d_st2[x]);
        if(rm_db->ey_2d_st2[x]) free(rm_db->ey_2d_st2[x]);
    }
    free(rm_db->den_2d_st2);
    free(rm_db->phi_2d_st2);
    free(rm_db->ex_2d_st2);
    free(rm_db->ey_2d_st2);
    free(rm_db->charge_ip_2d);
    free(rm_db->w_2d);
    free(rm_db->wx_2d_st);
    free(rm_db->wy_2d_st);
    free(rm_db->wx2_2d_st);
    free(rm_db->wy2_2d_st);
    free(rm_db);
}


void charge_fft_call_2d(fftDB_ptr fftDB)
{
    int n1 = fftDB->dft_bin_2d.x;
    int n2 = fftDB->dft_bin_2d.y;
    double a_den = 0.0;
    double a_phi = 0.0;
    double denom = 0.0;
    double a_ex = 0.0;
    double a_ey = 0.0;
    double wx = 0.0;
    double wx2 = 0.0;
    double wy = 0.0;
    double wy2 = 0.0;
    ddct2d(n1, n2, -1, fftDB->den_2d_st2, NULL, fftDB->charge_ip_2d, fftDB->w_2d);
    for (int x = 0; x < n1; x++) {
        fftDB->den_2d_st2[x][0] *= 0.5;
    }
    for (int y = 0; y < n2; y++) {
        fftDB->den_2d_st2[0][y] *= 0.5;
    }
    for (int x = 0; x < n1; x++) {
        for (int y = 0; y < n2; y++) {
            fftDB->den_2d_st2[x][y] *= 4.0 / (double)n1 / (double)n2;
        }
    }
    for (int x = 0; x < n1; x++)
    {
        wx = fftDB->wx_2d_st[x];
        wx2 = fftDB->wx2_2d_st[x];
        for (int y = 0; y < n2; y++)
        {
            wy = fftDB->wy_2d_st[y];
            wy2 = fftDB->wy2_2d_st[y];

            a_den = fftDB->den_2d_st2[x][y];
            if (x == 0 && y == 0){
                a_phi = 0.0;
                a_ex = 0.0;
                a_ey = 0.0;
            }
            else{
                denom = wx2 + wy2;
                a_phi = a_den / denom;
                a_ex = a_phi * wx;
                a_ey = a_phi * wy;
            }
            fftDB->phi_2d_st2[x][y] = a_phi;
            fftDB->ex_2d_st2[x][y] = a_ex;
            fftDB->ey_2d_st2[x][y] = a_ey;
        }
    }
    ddct2d(n1, n2, 1, fftDB->phi_2d_st2, NULL, fftDB->charge_ip_2d, fftDB->w_2d);
    ddsct2d(n1, n2, 1, fftDB->ex_2d_st2, NULL, fftDB->charge_ip_2d, fftDB->w_2d);
    ddcst2d(n1, n2, 1, fftDB->ey_2d_st2, NULL, fftDB->charge_ip_2d, fftDB->w_2d);
    // double a, b, c, d;
    // a = b = c = d = 0;
    // for (int x = 0; x < n1; x++)
    // {
    //     for (int y = 0; y < n2; y++)
    //     {
    //         a += fftDB->phi_2d_st2[x][y];
    //         b += fftDB->ex_2d_st2[x][y];
    //         c += fftDB->ey_2d_st2[x][y];
    //         d += fftDB->den_2d_st2[x][y];
    //     }
    // }
    // printf("a = %3f, b = %3f, c = %3f, d = %3f\n", a, b, c, d);
}

