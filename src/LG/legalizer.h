#pragma once
#include "legalization.h"

class Legalizer
{
    LegalizerInput * input;

    int getRowIdx(Cell const *cell);
    int getSubRowIdx(Row const *row, Cell const *cell);
    int placeRowTrail(int const &rowIdx, Cell *cell);
    void placeRowFinal(int const &rowIdx, int const &subRowIdx, Cell *cell);
    double calCost(Cell const *cell);
    void determinePosition();
    void abacusProcess();
    void calDisplacement();

    public:
        Legalizer(LegalizerInput *input) : input(input) {}
        void abacus();
};
