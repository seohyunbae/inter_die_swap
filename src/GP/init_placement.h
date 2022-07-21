#ifndef __INIT_PLACEMENT_H
#define __INIT_PLACEMENT_H

#include "../DB/global.h"
#include "../DB/DataBase.h"
#include "wlen.h"
#include <eigen3/Eigen/Core>
#include <Eigen/IterativeLinearSolvers>
#include <unsupported/Eigen/IterativeSolvers>
#include <Eigen/SparseCore>


using Eigen::VectorXf;
typedef Eigen::Triplet< float > T;
typedef Eigen::SparseMatrix< float, Eigen::RowMajor> SMatrix;

void initializePinOffsetCoord(dataBase_ptr data);
void placeAtCenter(dataBase_ptr data);
void initial_placement(dataBase_ptr data);
void update_inst(dataBase_ptr data, VectorXf &xcg_x, VectorXf &ycg_x);
void update_pin_by_inst(dataBase_ptr data);
void createSparseMatrix(VectorXf &xcg_x, VectorXf &xcg_b, 
                        VectorXf &ycg_x, VectorXf &ycg_b, 
                        SMatrix &eMatX,  SMatrix &eMatY,
                        dataBase_ptr data);
#endif