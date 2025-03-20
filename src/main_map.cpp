#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/simpleideals.h>


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
void printRingComponents(ring R)
{
    // Print the coefficient field
    coeffs cf = R->cf;
    std::cout << "Coefficient Field: " << n_GetChar(cf) << std::endl;

    // Print the variables
    std::cout << "Variables: ";
    for (int i = 0; i < R->N; ++i)
    {
        std::cout << R->names[i] << " ";
    }
    std::cout << std::endl;

    // Print the ordering
    std::cout << "Ordering: ";
    for (int i = 0; i < 2; ++i)
    {
        std::cout << (R->order[i]) << " ";
    }
    std::cout << std::endl;
}
int main()
{
    // Initialize Singular
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

     char *extNames[] = {(char *)"t(1)", (char *)"t(2)", (char *)"D"};
    char *varType = (char *)"z";
    ring R = createRing(extNames, 3, &varType, 9);
    rChangeCurrRing(R);
    std::cout << "Current ring: " << rString(currRing) << std::endl;
    
    // Define an ideal in R using generators with gen(k)
    ideal I = idInit(2, 1); // Initialize an ideal with 2 generators
    printRingComponents(R);

    coeffs cf = R->cf; // getting the coefficient field from the ring R

    // Create coefficients from the transcendental extension field
    number t1 = n_Param(1, cf); // t(1)
    number t2 = n_Param(2, cf); // t(2)

    // First generator: (t(1) + t(2)) * z(1) * gen(6)
    poly p1 = p_ISet(1, R);
    p_SetExp(p1, 1, 1, R); // z(1)
    p_SetComp(p1, 6, R);   // gen(6)
    p_Setm(p1, R);

    // Multiply by (t(1) + t(2))
    number sum = n_Add(t1, t2, cf);
    p1 = p_Mult_nn(p1, sum, R);

    // Second generator: t(1) * z(1) * gen(7) - t(2) * z(2) * gen(7)
    poly p2 = p_ISet(1, R);
    p_SetExp(p2, 1, 1, R); // z(1)
    p_SetComp(p2, 7, R);   // gen(7)
    p_Setm(p2, R);

    // Multiply by t(1)
    p2 = p_Mult_nn(p2, t1, R);

    poly p3 = p_ISet(1, R);
    p_SetExp(p3, 2, 1, R); // z(2)
    p_SetComp(p3, 7, R);   // gen(7)
    p_Setm(p3, R);

    // Multiply by t(2)
    p3 = p_Mult_nn(p3, t2, R);

    // Subtract p3 from p2
    p2 = p_Add_q(p2, p_Neg(p3, R), R);

    // Assign the generators to the ideal
    I->m[0] = p1;
    I->m[1] = p2;
    // Print the ideal to verify the components
    std::cout << "Ideal in R: " << std::endl;
    for (int i = 0; i < IDELEMS(I); i++)
    {
        std::cout << "I[" << i << "] = " << pString(I->m[i]) << std::endl;
    }

   char *extNames1[] = {(char *)"t(1)", (char *)"t(2)", (char *)"D"};
    char *varType1 = (char *)"x";
    ring r = createRing(extNames1, 3, &varType1, 9);
   
    rChangeCurrRing(r);
    std::cout << "Current second ring: " << rString(r) << std::endl;

    // Use identity coefficient mapping
    nMapFunc nMap = n_SetMap(R->cf, r->cf);

    int C = IDELEMS(I);

    const int *perm = NULL;

    // Map the ideal from `R` to `r`
    ideal I_mapped = id_PermIdeal(I, 1, C, perm, R, r, nMap, NULL, 1, FALSE);

    if (I_mapped != NULL)
    {
        int tt = IDELEMS(I_mapped);
        std::cout << "Mapped Ideal: " << std::endl;

        // Print
        for (int i = 0; i < tt; i++)
        {
            std::cout << "I_mapped->m[" << i << "]: " << p_String(I_mapped->m[i], r) << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: id_PermIdeal returned NULL." << std::endl;
    }

    // Free memory
    id_Delete(&I, R);
    id_Delete(&I_mapped, r);
    rDelete(R);
    rDelete(r);

    return 0;
}