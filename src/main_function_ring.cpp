#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"

ring createRing(char **extNames, int extCount, char **varNames, int varCount)
{
    // Create the coefficient field Q[t(1), t(2), D]
    ring extRing = rDefault(0, extCount, extNames);
    //std::cout << "Current ring: " << rString(extRing) << std::endl;

    TransExtInfo extParam;
    extParam.r = extRing;
    coeffs cf = nInitChar(n_transExt, &extParam);

    // Define variable names for the ring
    char *vars[varCount];
    for (int i = 0; i < varCount; ++i)
    {
        vars[i] = (char *)omAlloc(5 * sizeof(char));
        sprintf(vars[i], "%s(%d)", varNames[0], i + 1);
    }

    // Define the orders (dp(varCount), C)
    rRingOrder_t *order = (rRingOrder_t *)omAlloc0(3 * sizeof(rRingOrder_t));
    order[0] = ringorder_dp;
    order[1] = ringorder_c;

    // Define the blocks of the ring
    int *block0 = (int *)omAlloc(3 * sizeof(int));
    block0[0] = 1;
    int *block1 = (int *)omAlloc0(3 * sizeof(int));
    block1[0] = varCount;

    // Define the ring using rDefault function
    return rDefault(cf, varCount, vars, 3, order, block0, block1);
}

int main()
{
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    char *extNames[] = {(char *)"t(1)", (char *)"t(2)", (char *)"D"};
    char *varType = (char *)"z";
    ring R = createRing(extNames, 3, &varType, 9);
    rChangeCurrRing(R);
    std::cout << "Current ring: " << rString(currRing) << std::endl;
    return 0;
}
