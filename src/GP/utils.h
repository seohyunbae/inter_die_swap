#ifndef __REPLACE_UTIL__
#define __REPLACE_UTIL__ 0

#include <iostream>
#include "../DB/global.h"


unsigned prec2unsigned(double a);
void itoa(int n, char k[]);

void time_start(double *time_cost);
void time_end(double *time_cost);
void time_calc(double last_time, double *curr_time, double *time_cost);

std::string getexepath();

FPOS fp_mul(struct FPOS a, struct FPOS b);
inline FPOS fp_add(struct FPOS a, struct FPOS b) {
  struct FPOS c;
  c.x = a.x + b.x;
  c.y = a.y + b.y;
  return c;
}
FPOS fp_add_abs(struct FPOS a, struct FPOS b);
inline FPOS fp_scal(double s, struct FPOS a) {
  struct FPOS c = a;
  c.x *= s;
  c.y *= s;
  return c;
}

double fp_sum(struct FPOS a);
FPOS fp_subt(struct FPOS a, struct FPOS b);
FPOS fp_subt_const(struct FPOS a, double b);
FPOS fp_exp(struct FPOS a);
double fp_product(struct FPOS a);

int p_product(struct POS a);
int p_max(struct POS a);

FPOS fp_div(struct FPOS a, struct FPOS b);
FPOS fp_rand(void);
FPOS fp_inv(struct FPOS a);
FPOS p2fp(struct POS a);

double get_abs(double a);


// Rect common area functions
int iGetCommonAreaXY(POS aLL, POS aUR, POS bLL, POS bUR);
double pGetCommonAreaXY(FPOS aLL, FPOS aUR, FPOS bLL, FPOS bUR); 

// String replace functions
bool ReplaceStringInPlace(std::string &subject,
                                 const std::string &search,
                                 const std::string &replace);
void SetEscapedStr(std::string &inp);
char *GetEscapedStr(const char *name, bool isEscape = true);


// Print functions
void PrintProc(std::string input, int verbose = 0);
void PrintProcBegin(std::string input, int verbose = 0);
void PrintProcEnd(std::string input, int verbose = 0);
void PrintError(std::string input, int verbose = 0); 
void PrintInfoInt(std::string input, int val, int verbose = 0);
void PrintInfoPrec(std::string input, double val, int verbose = 0);
void PrintInfoPrecSignificant(std::string input, double val, int verbose = 0);
void PrintInfoPrecPair(std::string input, double val1, double val2, int verbose = 0);
void PrintInfoString(std::string input, int verbose = 0);
void PrintInfoString(std::string input, std::string val, int verbose = 0);
void PrintInfoRuntime(std::string input, double runtime, int verbose = 0);


// scaleDown vars / functions
// custom scale down parameter setting during the stage
void SetDefDbu(double _dbu);
void SetUnitX(float _unitX);
void SetUnitY(float _unitY);
void SetOffsetX(double _offsetX);
void SetOffsetY(double _offsetY);
void SetUnitY(double _unitY);

double GetUnitX();
double GetUnitY();
double GetOffsetX();
double GetOffsetY();
double GetDefDbu(); 

int GetScaleUpSize(double input);
int GetScaleUpPointX(double input);
int GetScaleUpPointY(double input);
double GetScaleUpPointdoubleX( double input);
double GetScaleUpPointdoubleY( double input);

double GetScaleDownSize(double input);
double GetScaleDownPoint(double input);

int calc_largest_po2(int number);


inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = {a};
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

inline double fastExp(double a) {
  a = 1.0 + a / 1024.0;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  a *= a;
  return a;
}

#endif