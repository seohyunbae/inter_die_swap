#ifndef _RESULT_WRITER_H
#define _RESULT_WRITER_H

#include "global.h"
#include "DataBase.h"

COORD get_left_lower_coordinate(FCOORD center, int const &sizeX, int const &sizeY);
void write(dataBase_ptr data, std::string const &filename);

#endif