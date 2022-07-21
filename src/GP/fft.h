#ifndef _FFT_H
#define _FFT_H


#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "utils.h"


struct FFT_DB{
    int* charge_ip_2d;
    int charge_dft_n_2d;
    int charge_dft_nbin_2d;
    int charge_dft_nbit_2d;
    int charge_dft_nw_2d;

    double **den_2d_st2;
    double **phi_2d_st2;
    double **e_2d_st2;
    double **ex_2d_st2;
    double **ey_2d_st2;
    double **ez_2d_st2;
    double *w_2d;
    double *wx_2d_st;
    double *wx2_2d_st;
    double *wy_2d_st;
    double *wy2_2d_st;
    double *wz_2d_st;
    double *wz2_2d_st;
    double *wx_2d_iL;
    double *wy_2d_iL;
    double *wx2_2d_iL;
    double *wy2_2d_iL;

    double DFT_SCALE_2D;
    struct POS dft_bin_2d;
    struct FPOS dft_bin_size;

}; typedef struct FFT_DB* fftDB_ptr;




/// Electrostatic-Based Functions /////////////////////////////////////////
struct POS getFFTCoord(fftDB_ptr fftDB, instance_ptr inst);
fftDB_ptr create_FFTDB(dataBase_ptr data);
void destroy_FFTDB(fftDB_ptr rm_db);
void charge_fft_call_2d(fftDB_ptr fftDB);

inline void copy_e_from_fft_2D(fftDB_ptr fftDB, struct FPOS *e, struct POS p) {
  e->x = fftDB->ex_2d_st2[p.x][p.y];
  e->y = fftDB->ey_2d_st2[p.x][p.y];
}

inline void copy_phi_from_fft_2D(fftDB_ptr fftDB, double *phi, struct POS p) {
  *phi = fftDB->phi_2d_st2[p.x][p.y];
}

inline void copy_den_to_fft_2D(fftDB_ptr fftDB, double den, struct POS p) {
  fftDB->den_2d_st2[p.x][p.y] = den;
}

/// 1D FFT ////////////////////////////////////////////////////////////////
void cdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);

/// 2D FFT ////////////////////////////////////////////////////////////////
void cdft2d(int, int, int, double **, double *, int *, double *);
void rdft2d(int, int, int, double **, double *, int *, double *);
void ddct2d(int, int, int, double **, double *, int *, double *);
void ddst2d(int, int, int, double **, double *, int *, double *);
void ddsct2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void ddcst2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);

#endif
