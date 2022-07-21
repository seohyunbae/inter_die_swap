#ifndef _PARSE_LINE_H
#define _PARSE_LINE_H

#define max_line_length 1024

#include "global.h"
#include "DataBase.h"

enum state{
    NumTechnologies, 
    Tech, 
    LibCell, 
    TechPin, 

    DieSize,
    TopDieMaxUtil, 
    BottomDieMaxUtil, 

    TopDieRows, 
    BottomDieRows, 

    TopDieTech, 
    BottomDieTech, 

    TerminalSize, 
    TerminalSpacing, 

    NumInstances, 
    Inst, 

    NumNets, 
    NetDefine, 
    NetPin
};


struct CharToken{
    char* stream;
    struct  CharToken* next;
}; typedef struct CharToken* charToken_ptr;


struct splitToken{
    char** split;
    int arrayLen;
}; typedef struct splitToken* splitToken_ptr;


//List of functions
charToken_ptr create_charToken(char* stream);
void destroy_token_chain(charToken_ptr rm_token);
splitToken_ptr split_line(char* line);
void destroy_splitToken(splitToken_ptr rm_token);
int get_state(char* header, int prev_state);
dataBase_ptr DataBase_init(char* filename);

#endif