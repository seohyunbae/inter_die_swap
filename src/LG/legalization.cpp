#include "legalization.h"
#include <iostream>
#include <fstream>

void LegalizerDB::readDie(die_ptr data, int dieNum)
{
    int numRows = data->repeatCount;
    int height = data->rowHeight;
    int siteWidth = 1;
    int siteNum = data->rowLength;
    int x = data->lowerLeftX;
    int y = data->lowerLeftY;
    std::cout << dieNum << " " << height << std::endl;
    this->updateDieSize(data->lowerLeftX, data->lowerLeftY, data->upperRightX, data->upperRightY);

    for (int i = 0; i < numRows; ++i)
    {
        y = height * i;
        auto row = new Row(siteWidth, height, y);
        row->subRows.emplace_back(new SubRow(x, x + siteWidth * siteNum));
        this->die[dieNum]->rows.emplace_back(row);
    }    
}

void LegalizerDB::readDieDB(dieDB_ptr data)
{   
    die_ptr top = data->top_die;
    die_ptr bot = data->bot_die;
    
    this->die.emplace_back(new LegalizerInput());
    this->die.emplace_back(new LegalizerInput());
    // Create rows for top die
    readDie(top, top_die_idx);

    // Create rows for bottom die
    readDie(bot, bot_die_idx);
}


void LegalizerDB::readInstanceDB(instanceDB_ptr data)
{
    int numInsts = data->numInsts;
    for (int i = 0; i < numInsts; ++i)
    {   
        int width = 0, height = 0;
        double x = 0, y = 0;
                
        instance_ptr inst = data->inst_array[i];
        std::string name = std::string(inst->instanceName);
        width = inst->sizeX, height = inst->sizeY;
        x = inst->center.x - width / 2;
        y = inst->center.y - height / 2;

        auto cell = new Cell(name, width, height, x, y);

        this->die[inst->dieNum]->cells.emplace_back(cell);
        this->cell_hash.emplace(name, cell);
    }
}

void LegalizerDB::readTerminalDB(terminalDB_ptr data)
{
    this->terminal = new LegalizerInput();
    int numTerminals = data->numTerminals;
    // terminal size
    int sizeX = data->sizeX, sizeY = data->sizeY;
    int spacing = data->spacing;
    int width = sizeX + spacing;
    int height = sizeY + spacing;

    for (int i = 0; i < numTerminals; ++i)
    {
        std::string name;
        double x = 0, y = 0;
        
        terminal_ptr terminal = data->term_array[i];
        name = terminal->netName;

        x = terminal->center.x - width / 2;
        y = terminal->center.y - height / 2;

        auto cell = new Cell(name, width, height, x, y);
        this->terminal->cells.emplace_back(cell);
        this->cell_hash.emplace(name, cell);
    }

    int numRows = (int)((this->height - 2 * spacing) / height);
    int numCols = (int)((this->width - 2 * spacing) / width);

    int y = 0;
    for (int i = 0; i < numRows; ++i)
    {
        y = spacing + height * i;
        auto row = new Row(1, height, y);
        row->subRows.emplace_back(new SubRow(spacing, this->width - spacing));
        this->terminal->rows.emplace_back(row);
    }

    data->terminal_grid = new terminalGrid(numRows, numCols, width, height);
    data->terminal_grid->generateTerminalGrid();
}

void LegalizerDB::readDB(dataBase_ptr data)
{
    readDieDB(data->dieDB);
    readInstanceDB(data->instanceDB);
    readTerminalDB(data->terminalDB);   
}

void LegalizerDB::readGP(std::string const &filename)
{
    using namespace std;
    ifstream fin(filename);
    string buff;

    while (getline(fin, buff))
    {
        if (buff.empty())
            continue;
        stringstream buffStream(buff);
        string name;
        buffStream >> name;

        if (name == "Top" || name == "Bot" || name == "Terminal") continue;

        double x = 0, y = 0;
        buffStream >> x >> y;
        try
        {
            this->cell_hash.at(name)->x = double(x);
            this->cell_hash.at(name)->y = double(y);
        }
        catch(const std::exception& e)
        {
            int width = 0, height = 0;
            auto cell = new Cell(name, width, height, x, y);

            this->terminal->cells.emplace_back(cell);
            this->cell_hash.emplace(name, cell);
        }
    }
}

void LegalizerDB::writeDB(dataBase_ptr data)
{
    int numInsts = data->instanceDB->numInsts;
    for (int i = 0; i < numInsts; ++i)
    {   
        std::string name;
        int width = 0, height = 0;
        double x = 0, y = 0;
           
        instance_ptr inst = data->instanceDB->inst_array[i];
        name = std::string(inst->instanceName);
        width = this->cell_hash.at(name)->width, height = this->cell_hash.at(name)->height;
        x = this->cell_hash.at(name)->optimalX + width / 2;
        y = this->cell_hash.at(name)->optimalY + height / 2;
        inst->center.x = x, inst->center.y = y;
    }
    int numTerminals = data->terminalDB->numTerminals;
    for (int i = 0; i < numTerminals; ++i)
    {
        std::string name;
        int width = 0, height = 0;
        double x = 0, y = 0;
        
        terminal_ptr terminal = data->terminalDB->term_array[i];
        name = terminal->netName;
        width = terminal->sizeX, height = terminal->sizeY;
        x = this->cell_hash.at(name)->optimalX + width / 2;
        y = this->cell_hash.at(name)->optimalY + height / 2;
        terminal->center.x = x;
        terminal->center.y = y;

        data->terminalDB->terminal_grid->updateTerminalGrid(terminal);
    }
    // data->terminalDB->terminal_grid->printGrid();
}

void legalize_local(LegalizerInput * input)
{
    Legalizer legalizer(input);
    legalizer.abacus();
}


void legalize(dataBase_ptr data, std::string const &filename)
{
    printf("\nPROC:  Abacus Legalization\n");
    LegalizerDB legalizer_db;
    legalizer_db.readDB(data);

    if (filename != "NULL")
    {        
        // Read GP results
        legalizer_db.readGP(filename);
    }

    // update_pin_by_inst(data);
    // update_net_pos(data);
    double HPWL = getUnscaledHPWL(data);
    // double HPWL = getHPWL(data);
    printf("Initial HPWL = %.3f\n", HPWL);

    printf("\nLegalize top die...\n");
    LegalizerInput *top_die = legalizer_db.die[top_die_idx];
    legalize_local(top_die);
    // print_placement(top_die);

    printf("\nLegalize bot die...\n");
    LegalizerInput *bot_die = legalizer_db.die[bot_die_idx];
    legalize_local(bot_die);
    // print_placement(bot_die);

    printf("\nLegalize terminal die...\n");
    LegalizerInput *terminal_die = legalizer_db.terminal;
    legalize_local(terminal_die);
    // print_placement(terminal_die);

    // update DB (instance, terminal)
    legalizer_db.writeDB(data);

    update_pin_by_inst(data);
    update_net_pos(data);
    HPWL = getUnscaledHPWL(data);

    printf("\nTotal HPWL = %.3f\n", HPWL);
}

void print_placement(LegalizerInput * input)
{
    using namespace std;
    for (auto cell : input->cells)
    {
        cout << "Name = " << cell->name << endl;
        cout << "Init placement: (" << cell->x << ", " << cell->y << ") ";
        cout << "Final placement: (" << cell->optimalX << ", " << cell->optimalY << ")" << endl;
    }
}
