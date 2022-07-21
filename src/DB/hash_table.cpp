#include "hash_table.h"


unsigned int hash_function(int cur_hash_table_len, char* name)
{
    unsigned int hash_index = 0;
    for (int i = 0; i < strlen(name); i++)
    {
        int cur_char = (int)(*(name + i));
        hash_index = hash_index * hash_key + cur_char;
    }
    return hash_index % cur_hash_table_len;
}
