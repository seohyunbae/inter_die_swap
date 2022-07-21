#include "FM_Die_DB.h"
#ifndef _PART_
#define _PART_

namespace FM_Die_Partition{

void print_state(FM& fm);

//for partitioning
bool basecell(FM_Instance* inst);
bool init_condition(FM& fm);

//for initial partitioning
void init_partition(FM& fm);

//for bi partitioning
void bi_partition(FM& fm);

//동작이 다 끝나고 DB로 다시 update
void update_DB(dataBase_ptr db, FM& fm);

//for debugging
void print_state(FM& fm); //fm state print
void simple_print(FM& fm); //cut 제대로 됐는지 die랑 cut만 간단하게 print
bool size_check(FM& fm); //die의 size가 제대로 반영되었는지 check
bool mincut_check(FM& fm); //best state로 돌린 이후에 cut이 제대로 반영되었는지 check
bool normal_path(FM& fm); //path에서 오류가 발생하지 않았는지 test하는 함수.
void tech_check(FM& fm);

//whole module
void Die_partition(dataBase_ptr db);
}
#endif