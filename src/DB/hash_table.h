#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>


#define default_hash_size   1024
#define hash_key            66337 //A prime number for hash table


//List of functions
unsigned int hash_function(int cur_hash_table_len, char* name);

#endif