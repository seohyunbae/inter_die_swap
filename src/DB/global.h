#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "utils/useful.h"
// shchung
#include <algorithm>
#include <limits>


//For C style coding
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "hash_table.h"

//For fm die partition
#include <map>
#include <list>
#include <climits>
#include <cmath>
#include <ctime>
#include <limits>
#include <cfloat>
#include <sys/time.h>


#define top_die_idx  0
#define bot_die_idx  1


#define MIN_LEN 25.0 // 10.0 // 5.0 // 1.0
#define Epsilon 1.0E-12
#define NumThreads 16
// #define initPlacementMaxIter 30
#define initPlacementMaxIter 30
#define PI 3.141592653589793238462L


#define BUF_SZ 511
#define MIN_AREA3 /* 1.0 */ 1.0e-12
#define LS_DEN
#define DetailPlace
#define MSH_Z_RES /* 8 */ 1
#define THETA_XY_3D_PLOT PI / 6.0
#define Z_SCAL 1.00

#define DEN_GRAD_SCALE 1.0 /* 0.1 */ /* 0.5 */ /* 0.25 */ /* 0.125 */
#define LAYER_ASSIGN_3DIC MIN_TIER_ORDER       



#define INT_CONVERT(a) (int)(1.0 * (a) + 0.5f)
#define INT_DOWN(a) (int)(a)
#define INT_UP(a) (int)(a) + 1
#define UNSIGNED_CONVERT(a) (unsigned)(1.0 * (a) + 0.5f)

#define IS_PRECISION_EQUAL(a, b) (fabs((a)-(b)) <= PREC_EPSILON)

#define minBinNumStepX 100
#define minBinNumStepY 100
#define binSizeX    200
#define binSizeY    200


#define dftBinSizeX /*1*/   /*2*/   /*4*/    8
#define dftBinSizeY /*1*/   /*2*/   /*4*/    8
#define dftBinScale /*1*/   /*2*/   /*4*/    8


#define chargeIncludeTerm   true
#define wlenIncludeTerm     true


#define ALPHA   1e-6

#define wlen_cof_x 1e-4
#define wlen_cof_y 1e-4
#define NEG_MAX_EXP -300


// For opt.h
// #define OPT_VERBOSE
// #define charge_weight 5e-7
// #define wlen_weight 1e-4
#define GP_epsilon 1e-12

// For nesterov.h
#define NS_VERBOSE
#define PLOT_INSTANCE
#define OVFL_MODE
// #define PIPE_LG


struct FCOORD{
    double x;
    double y;
}; typedef struct FCOORD* fCoord_ptr;


struct COORD{
    int x;
    int y;
}; typedef struct COORD* coord_ptr;


struct FPOS{
    double x;
    double y;
}; typedef struct FPOS* fPos_ptr;


struct POS{
    int x;
    int y;
}; typedef struct POS* pos_ptr;


struct FSIZE{
    double x;
    double y;
}; typedef struct FSIZE* fSize_ptr;


struct SIZE{
    int x;
    int y;
}; typedef struct SIZE* size_ptr;


struct FBBOX{
    struct FPOS pmin;
    struct FPOS pmax;
}; typedef struct FBBOX* fbbox_ptr;


struct BBOX{
    struct POS pmin;
    struct POS pmax;
}; typedef struct BBOX* bbox_ptr;

//for bin based fm
namespace FM_Die{
    class Bin;
}

// typedef double prec;
