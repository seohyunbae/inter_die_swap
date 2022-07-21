#include "InputParser.h"

using namespace std;


charToken_ptr create_charToken(char* stream)
{
    charToken_ptr new_token = (charToken_ptr)malloc(sizeof(struct CharToken));
    new_token->stream = strdup(stream);
    new_token->next = NULL;
    return new_token;
}

void destroy_token_chain(charToken_ptr rm_token)
{
    while(rm_token)
    {
        charToken_ptr token_next = rm_token->next;
        free(rm_token);
        rm_token = token_next;
    }
}


splitToken_ptr split_line(char* line)
{
    char *temp_line = strdup(line);
    int cnt = 1;
    char *token = strtok(temp_line, " ");
    charToken_ptr head = create_charToken(token);
    while (token)
    {
        token = strtok(NULL, " ");
        if (!token) break;
        charToken_ptr node = create_charToken(token);
        charToken_ptr sweep = head;
        while (sweep->next){
            sweep = sweep->next;
        }
        cnt++;
        sweep->next = node;
    }
    char** split_array = (char**)malloc(sizeof(char*) * cnt);
    charToken_ptr sweep = head;
    for (int i = 0; i < cnt; i++)
    {
        split_array[i] = sweep->stream;
        sweep = sweep->next;
    }

    *(split_array[cnt-1]+(strlen(split_array[cnt - 1]) - 1)) = '\0';
    free(temp_line);
    destroy_token_chain(head);
    splitToken_ptr splitToken = (splitToken_ptr)malloc(sizeof(struct splitToken));
    splitToken->arrayLen = cnt;
    splitToken->split = split_array;
    return splitToken;
}


void destroy_splitToken(splitToken_ptr rm_token)
{
    for (int i = 0; i < rm_token->arrayLen; i++)
    {
        free(rm_token->split[i]);
    }
    free(rm_token->split);
    free(rm_token);
}




int get_state(char* header, int prev_state)
{
    if (!strcmp(header, "NumTechnologies")) return NumTechnologies;
    else if (!strcmp(header, "Tech")) return Tech;
    else if (!strcmp(header, "LibCell")) return LibCell;
    else if (!strcmp(header, "Pin") && (prev_state == LibCell || prev_state == TechPin) ) return TechPin;
    else if (!strcmp(header, "DieSize")) return DieSize;
    else if (!strcmp(header, "TopDieMaxUtil")) return TopDieMaxUtil;
    else if (!strcmp(header, "BottomDieMaxUtil")) return BottomDieMaxUtil;
    else if (!strcmp(header, "TopDieRows")) return TopDieRows;
    else if (!strcmp(header, "BottomDieRows")) return BottomDieRows;
    else if (!strcmp(header, "TopDieTech")) return TopDieTech;
    else if (!strcmp(header, "BottomDieTech")) return BottomDieTech;
    else if (!strcmp(header, "TerminalSize")) return TerminalSize;
    else if (!strcmp(header, "TerminalSpacing")) return TerminalSpacing;
    else if (!strcmp(header, "NumInstances")) return NumInstances;
    else if (!strcmp(header, "Inst")) return Inst;
    else if (!strcmp(header, "NumNets")) return NumNets;
    else if (!strcmp(header, "Net")) return NetDefine;
    else if (!strcmp(header, "Pin") && (prev_state == NetDefine || prev_state == NetPin) ) return NetPin;
    return -1;
}


dataBase_ptr DataBase_init(char* filename)
{
    FILE* target_file = fopen(filename, "r");
    if (target_file == NULL) return NULL;
    char line_array[max_line_length];
    char* line = &line_array[0];
    // char* line = (char*)calloc(sizeof(char), max_line_length);
    
    int state = -1;
    int next_state = -1;

    //Get first line
    line = fgets(line, max_line_length, target_file);
    while (line && line[0] < 32) line = fgets(line, max_line_length, target_file); // Skip Enters. 
    splitToken_ptr Token = split_line(line);
    char** splitted = Token->split;
    int cur_state = get_state(splitted[0], state);


    dataBase_ptr DB = create_DataBase();

    //========== Variables that are going to be used for parsing ============
    // Mastercell variables
    char* masterCellName;
    char* techName;
    char* pinName;
    int numPins, sizeX, sizeY, offsetX, offsetY;

    // Die variables
    string top = "TopDie";
    string bot = "BotDie";
    char* topDieName = (char*)top.c_str();
    char* botDieName = (char*)bot.c_str();
    int top_die_cnt = 0;
    int bot_die_cnt = 0;
    int lowerLeftX, lowerLeftY, upperRightX, upperRightY;
    char* top_dieTechName;
    char* bot_dieTechName;
    int top_targetUtil, top_startX, top_startY, top_rowLength, top_rowHeight, top_repeatCount;
    int bot_targetUtil, bot_startX, bot_startY, bot_rowLength, bot_rowHeight, bot_repeatCount;

    // Terminal variables
    int terminalSizeX, terminalSizeY, terminalSpacing;

    // Instance variables
    int numInst;
    char* instanceName;
    char* instanceMasterCellName;

    // Net Variables
    char* netName;
    char* netInstName;
    char* netInstPinName;
    int net_numPins;

    while (line)
    {
        switch (cur_state)
        {
            case NumTechnologies:
                construct_MasterCellDB(DB, atoi(splitted[1]));
                break;
            case Tech:
                techName = strdup(splitted[1]);
                break;
            case LibCell:
                masterCellName = strdup(splitted[1]);
                sizeX = atoi(splitted[2]);
                sizeY = atoi(splitted[3]);
                numPins = atoi(splitted[4]);
                add_masterCell(DB, masterCellName, techName, numPins, sizeX, sizeY);
                break;
            case TechPin:
                pinName = splitted[1];
                offsetX = atoi(splitted[2]);
                offsetY = atoi(splitted[3]);
                add_pin(DB, masterCellName, techName, pinName, offsetX, offsetY);
                // If next one is not pin: free techName, masterCellName
                break;

            case DieSize:
                lowerLeftX = atoi(splitted[1]);
                lowerLeftY = atoi(splitted[2]);
                upperRightX = atoi(splitted[3]);
                upperRightY = atoi(splitted[4]);
                construct_dieDB(DB, lowerLeftX, lowerLeftY, upperRightX, upperRightY);
                break;
            case TopDieMaxUtil:
                top_die_cnt++;
                top_targetUtil = atoi(splitted[1]);
                if (top_die_cnt == 3) {
                    create_die(DB, 1, topDieName, top_dieTechName, top_targetUtil, top_startX, top_startY, top_rowLength, top_rowHeight, top_repeatCount);
                    free(top_dieTechName);
                }
                break;
            case BottomDieMaxUtil:
                bot_die_cnt++;
                bot_targetUtil = atoi(splitted[1]);
                if (bot_die_cnt == 3) {
                    create_die(DB, 0, botDieName, bot_dieTechName, bot_targetUtil, bot_startX, bot_startY, bot_rowLength, bot_rowLength, bot_repeatCount);
                    free(bot_dieTechName);
                }
                break;

            case TopDieRows:
                top_die_cnt++;
                top_startX = atoi(splitted[1]);
                top_startY = atoi(splitted[2]);
                top_rowLength = atoi(splitted[3]);
                top_rowHeight = atoi(splitted[4]);
                top_repeatCount = atoi(splitted[5]);
                if (top_die_cnt == 3) {
                    create_die(DB, 1, topDieName, top_dieTechName, top_targetUtil, top_startX, top_startY, top_rowLength, top_rowHeight, top_repeatCount);
                    free(top_dieTechName);
                }
                break;
            case BottomDieRows:
                bot_die_cnt++;
                bot_startX = atoi(splitted[1]);
                bot_startY = atoi(splitted[2]);
                bot_rowLength = atoi(splitted[3]);
                bot_rowHeight = atoi(splitted[4]);
                bot_repeatCount = atoi(splitted[5]);
                if (bot_die_cnt == 3) {
                    create_die(DB, 0, botDieName, bot_dieTechName, bot_targetUtil, bot_startX, bot_startY, bot_rowLength, bot_rowHeight, bot_repeatCount); // shchung
                    free(bot_dieTechName);
                }
                break;

            case TopDieTech:
                top_die_cnt++;
                top_dieTechName = strdup(splitted[1]);
                if (top_die_cnt == 3) {
                    create_die(DB, 1, topDieName, top_dieTechName, top_targetUtil, top_startX, top_startY, top_rowLength, top_rowHeight, top_repeatCount);
                    free(top_dieTechName);
                }
                break;
            case BottomDieTech:
                bot_die_cnt++;
                bot_dieTechName = strdup(splitted[1]);
                if (bot_die_cnt == 3) { 
                    create_die(DB, 0, botDieName, bot_dieTechName, bot_targetUtil, bot_startX, bot_startY, bot_rowLength, bot_rowHeight, bot_repeatCount); // shchung
                    free(bot_dieTechName);
                }
                break;

            case TerminalSize:
                terminalSizeX = atoi(splitted[1]);
                terminalSizeY = atoi(splitted[2]);
                cur_state = TerminalSpacing;
                break;
            case TerminalSpacing:
                terminalSpacing = atoi(splitted[1]);
                construct_terminalDB(DB, terminalSizeX, terminalSizeY, terminalSpacing);
                break;

            case NumInstances:
                numInst = atoi(splitted[1]);
                construct_instanceDB(DB, numInst);
                cur_state = Inst;
                break;
            case Inst:
                instanceName = splitted[1];
                instanceMasterCellName = splitted[2];
                add_instance(DB, instanceName, instanceMasterCellName);
                break;
            case NumNets:
                construct_netDB(DB, atoi(splitted[1]));
                cur_state = NetDefine;
                break;
            case NetDefine:
                netName = strdup(splitted[1]);
                net_numPins = atoi(splitted[2]);
                add_net(DB, netName, net_numPins);
                break;
            case NetPin:
                char* find_slash = strstr(splitted[1], "/");
                *find_slash = '\0';
                netInstName = splitted[1];
                netInstPinName = find_slash + 1;
                link_net_inst(DB, netName, netInstName, netInstPinName);
                //When going next it must free netName
                break;
        }
        destroy_splitToken(Token);
        line = fgets(line, max_line_length, target_file);
        if(line == NULL) break;
        while (line[0] < 32) line = fgets(line, max_line_length, target_file);
        Token = split_line(line);
        splitted = Token->split;
        next_state = get_state(splitted[0], cur_state);
        if (cur_state == TechPin)
        {
            if (next_state != TechPin)
            {
                if(next_state!=LibCell) free(techName);
                free(masterCellName);
            }
        }
        else if (cur_state == NetPin)
        {
            if (next_state != NetPin) {
                free(netName);
            }
        }
        cur_state = next_state;
    }
    if(cur_state == NetPin) free(netName);
    free(line);
    fclose(target_file);
    return DB;
}