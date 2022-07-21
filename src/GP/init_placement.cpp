#include "init_placement.h"
#include <omp.h>


void initializePinOffsetCoord(dataBase_ptr data)
{
    // This is to initialize pin offset coordinate. 
    // Since it needs Die.h, Tech.h, Instance.h, it was better to initialize here. 
    inst2net_ptr sweep_inst_pin;
    cellSpec_ptr topTech, botTech;
    instance_ptr curInst;
    pin_ptr sweep_pin;
    for(int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];
        topTech = get_cellSpec(data, curInst->masterCellName, data->dieDB->top_die->techName);
        botTech = get_cellSpec(data, curInst->masterCellName, data->dieDB->bot_die->techName);
        sweep_inst_pin = curInst->net_head;
        while(sweep_inst_pin)
        {
            sweep_pin = topTech->pin_head;
            while(sweep_pin)
            {
                if(!strcmp(sweep_pin->pinName, sweep_inst_pin->pinName))
                {
                    sweep_inst_pin->topOffsetCoord.x = sweep_pin->offsetX;
                    sweep_inst_pin->topOffsetCoord.y = sweep_pin->offsetY;

                    break;
                }
                sweep_pin = sweep_pin->next;
            }
            sweep_pin = botTech->pin_head;
            while(sweep_pin)
            {
                if(!strcmp(sweep_pin->pinName, sweep_inst_pin->pinName))
                {
                    sweep_inst_pin->botOffsetCoord.x = sweep_pin->offsetX;
                    sweep_inst_pin->botOffsetCoord.y = sweep_pin->offsetY;
                    break;
                }
                sweep_pin = sweep_pin->next;
            }
            if (curInst->dieNum == 0) {
                sweep_inst_pin->curOffsetCoord.x = sweep_inst_pin->topOffsetCoord.x;
                sweep_inst_pin->curOffsetCoord.y = sweep_inst_pin->topOffsetCoord.y;
            }
            else if (curInst->dieNum == 1) {
                sweep_inst_pin->curOffsetCoord.x = sweep_inst_pin->botOffsetCoord.x;
                sweep_inst_pin->curOffsetCoord.y = sweep_inst_pin->botOffsetCoord.y;
            }
            sweep_inst_pin = sweep_inst_pin->next;
        }
    }
    return;
}


void initial_placement(dataBase_ptr data)
{
    using namespace Eigen;
    printf("PROC:  Conjugate Gradient (CG) method to obtain the IP\n");

    initializePinOffsetCoord(data);
    placeAtCenter(data);
    update_net_pos(data);
    int maxIter = 100;

    double target_tol = 0.000001;
    double x_err = 0; 
    double y_err = 0;

    double time_s = 0;
    
    // double HPWL = getHPWL(data);
    double HPWL = getUnscaledHPWL(data);
    printf("Initial HPWL = %3f\n", HPWL);
    printf("INFO: The Matrix Size id %d\n", data->instanceDB->numInsts);
    fflush(stdout);

    // omp_set_num_threads(NumThreads);
    setNbThreads(NumThreads);

    int totalNumInst = data->instanceDB->numInsts + data->terminalDB->numTerminals;
    SMatrix eMatX(totalNumInst, totalNumInst);
    SMatrix eMatY(totalNumInst, totalNumInst);

    VectorXf xcg_x(totalNumInst), xcg_b(totalNumInst);
    VectorXf ycg_x(totalNumInst), ycg_b(totalNumInst);

    struct timeval begin, end;
    printf("numThreads %d\n", nbThreads());
    for (int i = 0;; i++){
        if (i >= initPlacementMaxIter) break;
        gettimeofday(&begin, 0);
        createSparseMatrix(xcg_x, xcg_b, ycg_x, ycg_b, eMatX, eMatY, data);
        BiCGSTAB< SMatrix, IdentityPreconditioner > solver;
        solver.setMaxIterations(maxIter);
        solver.compute(eMatX);
        xcg_x = solver.solveWithGuess(xcg_b, xcg_x);
        x_err = solver.error();

        solver.compute(eMatY);
        ycg_x = solver.solveWithGuess(ycg_b, ycg_x);
        y_err = solver.error();
        update_inst(data, xcg_x, ycg_x);
        update_pin_by_inst(data);
        update_net_pos(data);

        HPWL = getUnscaledHPWL(data);
        // HPWL = getHPWL(data);
        
        // setNbThreads(0);
        gettimeofday(&end, 0);
        printf("INFO: IP %d, XCG Error %.6lf, YCG Error %.6lf HPWL %.6lf, Time %.6lf\n\n", 
                i, x_err, y_err, HPWL, (end.tv_sec - begin.tv_sec + (end.tv_usec - end.tv_usec) * 10E-6));
        fflush(stdout);
        if (fabs(x_err) < target_tol && fabs(y_err) < target_tol && i > 4) {
            break;
        }
    }
}


void update_inst(dataBase_ptr data, VectorXf &xcg_x, VectorXf &ycg_x)
{
    instance_ptr curInst = NULL;
    terminal_ptr curTerm = NULL;
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];
        curInst->center.x = xcg_x(i);
        curInst->center.y = ycg_x(i);
        
        //When reached edge
        if (curInst->center.x + 0.5 * (double)curInst->sizeX > data->dieDB->upperRightX) {
            curInst->center.x = (double)data->dieDB->upperRightX - 0.5 * (double)curInst->sizeX - Epsilon;
        }
        if (curInst->center.y + 0.5 * (double)curInst->sizeY > data->dieDB->upperRightY) {
            curInst->center.y = (double)data->dieDB->upperRightY - 0.5 * (double)curInst->sizeY - Epsilon;
        }
        if (curInst->center.x - 0.5 * (double)curInst->sizeX < data->dieDB->lowerLeftX) {
            curInst->center.x = (double)data->dieDB->lowerLeftX + 0.5 * (double)curInst->sizeX + Epsilon;
        }
        if (curInst->center.y - 0.5 * (double)curInst->sizeY < data->dieDB->lowerLeftY) {
            curInst->center.y = (double)data->dieDB->lowerLeftY + 0.5 * (double)curInst->sizeY + Epsilon;
        }
    }
    for (int i = data->instanceDB->numInsts; i < data->instanceDB->numInsts + data->terminalDB->numTerminals; i++)
    {
        curTerm = data->terminalDB->term_array[i - data->instanceDB->numInsts];
        curTerm->center.x = xcg_x(i);
        curTerm->center.y = ycg_x(i);

        if (curTerm->center.x + 0.5 * (double)curTerm->sizeX + (double)curTerm->spacing > data->dieDB->upperRightX){
            curTerm->center.x = (double)data->dieDB->upperRightX - 0.5 * (double)curTerm->sizeX - (double)curTerm->spacing - Epsilon;
        }
        if (curTerm->center.y + 0.5 * (double)curTerm->sizeY + (double)curTerm->spacing > data->dieDB->upperRightY){
            curTerm->center.y = (double)data->dieDB->upperRightY - 0.5 * (double)curTerm->sizeY - (double)curTerm->spacing - Epsilon;
        }
        if (curTerm->center.x - 0.5 * (double)curTerm->sizeX - (double)curTerm->spacing < data->dieDB->lowerLeftX){
            curTerm->center.x = (double)data->dieDB->lowerLeftX + 0.5 * (double)curTerm->sizeX + (double)curTerm->spacing;
        }
        if (curTerm->center.y - 0.5 * (double)curTerm->sizeY - (double)curTerm->spacing < data->dieDB->lowerLeftY){
            curTerm->center.y = (double)data->dieDB->lowerLeftY + 0.5 * (double)curTerm->sizeY + (double)curTerm->spacing;
        }
    }
}


void update_pin_by_inst(dataBase_ptr data)
{
    instance_ptr curInst;
    inst2net_ptr sweep_instLink;
    net2inst_ptr sweep_netLink;
    FCOORD inst_bottomLeftCoord;
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];
        sweep_instLink = curInst->net_head;
        inst_bottomLeftCoord.x = curInst->center.x - curInst->sizeX / 2.0;
        inst_bottomLeftCoord.y = curInst->center.y - curInst->sizeY / 2.0;
        while(sweep_instLink)
        {
            sweep_instLink->curCoord.x = inst_bottomLeftCoord.x + sweep_instLink->curOffsetCoord.x;
            sweep_instLink->curCoord.y = inst_bottomLeftCoord.y + sweep_instLink->curOffsetCoord.y;

            sweep_netLink = (net2inst_ptr)sweep_instLink->to_netLink;
            sweep_netLink->curCoord.x = sweep_instLink->curCoord.x;
            sweep_netLink->curCoord.y = sweep_instLink->curCoord.y;
            sweep_instLink = sweep_instLink->next;
        }
    }
}


void placeAtCenter(dataBase_ptr data)
{
    // #pragma omp parallel default(none) shared(data)
    // #pragma omp for
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        instance_ptr    curInst;
        inst2net_ptr    sweepInstLink;
        net2inst_ptr    netLink;
        net_ptr         curNet;
        terminal_ptr    curTerm;
        FCOORD instBotLeft;
        curInst = data->instanceDB->inst_array[i];
        curInst->center.x = data->dieDB->centerX;
        curInst->center.y = data->dieDB->centerY;
        // Update instance coord when it's at top die
        if (!curInst->dieNum) // Update
        {
            instBotLeft.x = curInst->center.x - curInst->sizeX / 2.0;
            instBotLeft.y = curInst->center.y - curInst->sizeY / 2.0;
            sweepInstLink = curInst->net_head;
            while(sweepInstLink)
            {
                netLink = (net2inst_ptr)sweepInstLink->to_netLink;
                sweepInstLink->curCoord.x = instBotLeft.x + sweepInstLink->topOffsetCoord.x;
                sweepInstLink->curCoord.y = instBotLeft.y + sweepInstLink->topOffsetCoord.y;

                netLink->curCoord.x = sweepInstLink->curCoord.x;
                netLink->curCoord.y = sweepInstLink->curCoord.y;
                sweepInstLink = sweepInstLink->next;
            }
        }
        // Update instance coord when it's at bot die
        else
        {
            instBotLeft.x = curInst->center.x - curInst->sizeX / 2.0;
            instBotLeft.y = curInst->center.y - curInst->sizeY / 2.0;
            sweepInstLink = curInst->net_head;
            while(sweepInstLink)
            {
                netLink = (net2inst_ptr)sweepInstLink->to_netLink;
                sweepInstLink->curCoord.x = instBotLeft.x + sweepInstLink->botOffsetCoord.x;
                sweepInstLink->curCoord.y = instBotLeft.y + sweepInstLink->botOffsetCoord.y;

                netLink->curCoord.x = sweepInstLink->curCoord.x;
                netLink->curCoord.y = sweepInstLink->curCoord.y;
                sweepInstLink = sweepInstLink->next;
            }
        }
    }
    for (int i = 0; i < data->netDB->numNets; i++)
    {
        net_ptr         curNet;
        terminal_ptr    curTerm;
        curNet = data->netDB->net_array[i];
        net2inst_ptr pin;
        bool check;
        int dieNum;
        // int count = 0;
        for (int j = 0; j < curNet->numPins; j++)
        {
            if (j > 0 && dieNum != curNet->pin_array[j]->instance->dieNum)
            {
                if (curNet->cut == false) 
                {
                    printf("Net %s cut\n", curNet->netName);
                    // count++;
                    curNet->cut = true;
                }
            }
            dieNum = curNet->pin_array[j]->instance->dieNum;
        }
        // if (count > 0) exit(1);
        if (curNet->cut)
        {
            add_terminal(data, data->terminalDB->sizeX, data->terminalDB->sizeY, data->terminalDB->spacing, 
                         curNet);
            curTerm = (terminal_ptr)curNet->terminal;
            curTerm->center.x = data->dieDB->centerX;
            curTerm->center.y = data->dieDB->centerY;
        }
    }
}


void createSparseMatrix(VectorXf &xcg_x, VectorXf &xcg_b, 
                        VectorXf &ycg_x, VectorXf &ycg_b, 
                        SMatrix &eMatX,  SMatrix &eMatY,
                        dataBase_ptr data)
{
    int numPins = 0;
    int moduleID1 = 0;
    int moduleID2 = 0;
    double common1;
    double common2;

    using namespace std;

    net_ptr tempNet= NULL;
    net2inst_ptr pin1 = NULL;
    net2inst_ptr pin2 = NULL;

    instance_ptr inst1 = NULL;
    instance_ptr inst2 = NULL;

    FCOORD center1, center2;
    FCOORD fp1, fp2;

    vector < T > tripletListX, tripletListY;
    tripletListX.reserve(100000000);
    tripletListY.reserve(100000000);

    // Initialization. 
    instance_ptr curInst = NULL;
    terminal_ptr curTerm = NULL;
    for (int i = 0; i < data->instanceDB->numInsts; i++)
    {
        curInst = data->instanceDB->inst_array[i];

        xcg_x(i) = curInst->center.x;
        ycg_x(i) = curInst->center.y;

        xcg_b(i) = 0;
        ycg_b(i) = 0;
    }
    for (int i = data->instanceDB->numInsts; i < data->instanceDB->numInsts + 
                                                data->terminalDB->numTerminals; i++)
    {
        curTerm = data->terminalDB->term_array[i - data->instanceDB->numInsts];
        xcg_x(i) = curTerm->center.x;
        ycg_x(i) = curTerm->center.y;

        xcg_b(i) = 0;
        ycg_b(i) = 0;
    }

    for (int i = 0; i < data->netDB->numNets; i++)
    {
        tempNet = data->netDB->net_array[i];
        numPins = tempNet->numPins;
        common1 = 1.0 / ((double)numPins - 1.0);

        for (int j = 0; j < numPins; j++)
        {
            pin1 = tempNet->pin_array[j];
            moduleID1 = pin1->instance->instIndex;
            fp1 = pin1->curCoord;

            inst1 = pin1->instance;
            center1.x = inst1->center.x;
            center1.y = inst1->center.y;

            for (int k = j + 1; k < numPins + 1; k++)
            {
                if (k == numPins)
                {
                    if (tempNet->cut == false) break;
                    else // For terminal
                    {
                        curTerm = (terminal_ptr)tempNet->terminal;
                        moduleID2 = data->instanceDB->numInsts + curTerm->termIndex;
                        fp2 = curTerm->center;

                        center2.x = curTerm->center.x;
                        center2.y = curTerm->center.y;

                    }
                }
                else
                {
                    pin2 = tempNet->pin_array[k];
                    moduleID2 = pin2->instance->instIndex;
                    fp2 = pin2->curCoord;
                    
                    inst2 = pin2->instance;
                    center2.x = inst2->center.x;
                    center2.y = inst2->center.y;
                }

                // Update X coords
                double len_x = fabs(fp1.x - fp2.x);
                double wt_x = 0.0f;
                
                if (len_x >= MIN_LEN) wt_x = common1 / len_x;
                else wt_x = common1 / MIN_LEN;
                common2 = (-1.0) * wt_x;

                tripletListX.push_back(T(moduleID1, moduleID1, wt_x));
                tripletListX.push_back(T(moduleID2, moduleID2, wt_x));
                tripletListX.push_back(T(moduleID1, moduleID2, common2));
                tripletListX.push_back(T(moduleID2, moduleID1, common2));

                xcg_b(moduleID1) += 
                    common2 * ((fp1.x - center1.x) - (fp2.x - center2.x));
                xcg_b(moduleID2) += 
                    common2 * ((fp2.x - center2.x) - (fp1.x - center1.x));

                // Update Y coords
                double len_y = fabs(fp1.y - fp2.y);
                double wt_y = 0.0f;
                
                if (len_y >= MIN_LEN) wt_y = common1 / len_y;
                else wt_y = common1 / MIN_LEN;
                common2 = (-1.0) * wt_y;
                
                tripletListY.push_back(T(moduleID1, moduleID1, wt_y));
                tripletListY.push_back(T(moduleID2, moduleID2, wt_y));
                tripletListY.push_back(T(moduleID1, moduleID2, common2));
                tripletListY.push_back(T(moduleID2, moduleID1, common2));
                
                ycg_b(moduleID1) += 
                    common2 * ((fp1.y - center1.y) - (fp2.y - center2.y));
                ycg_b(moduleID2) += 
                    common2 * ((fp2.y - center2.y) - (fp1.y - center1.y));
            }
        }
    }
    eMatX.setFromTriplets(tripletListX.begin(), tripletListX.end());
    eMatY.setFromTriplets(tripletListY.begin(), tripletListY.end());
}