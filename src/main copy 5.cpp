#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"

int main() {
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

   char **n = (char**)omalloc(3 * sizeof(char*));
n[0] = omStrDup("t(1)");
n[1] = omStrDup("t(2)");
n[2] = omStrDup("D");

// Create the ring Z/32003[x,y,z]
ring extRing = rDefault(0, 3, n);

// Define coefficient information
TransExtInfo extParam;
extParam.r = extRing;
coeffs cf = nInitChar(n_transExt, &extParam);
const int numVars = 9;       // z(1) to z(9)

// Dynamically create the names for the variables z(1) to z(9)
char *vars[numVars];
for (int i = 0; i < numVars; ++i) {
    vars[i] = (char*)omAlloc(5 * sizeof(char)); // "z(i)" where i = 1,2,...,9
    sprintf(vars[i], "z(%d)", i + 1);
}

// Define the orders (dp(9), C)
rRingOrder_t* order = (rRingOrder_t*)omAlloc0(3 * sizeof(rRingOrder_t));
order[0] = ringorder_dp;  // dp(9)
order[1] = ringorder_c;   // C (for the coefficient field)

// Define the blocks of the ring
int* block0 = (int*)omAlloc(3 * sizeof(int));
block0[0] = 1;  
int* block1 = (int*)omAlloc0(3 * sizeof(int));
block1[0] = 9;  

// Define the ring using rDefault function
ring R = rDefault(cf, numVars, vars, 3, order, block0, block1);  // 0 means coefficient field is ℚ

    rChangeCurrRing(R);
    std::cout << "Current first ring: " << rString(currRing) << std::endl;

    // Define an ideal in R using generators with gen(k)
    ideal I = idInit(2, 1);  // Initialize an ideal with 2 generators

    // First generator: (t(1) + t(2)) * x(1) * gen(6) + 2 * t(2) * x(1) * gen(1)
    poly p1 = p_ISet(1, R);
    p_SetExp(p1, 1, 2, R);  // t(1) * t(2)
    p_SetExp(p1, 1, 1, R);  // x(1)
    p_SetComp(p1, 6, R);    // gen(6)
    p_Setm(p1, R);

    // Second generator: t(1) * x(1) * gen(7) - t(2) * x(2) * gen(7) ...
    poly p2 = p_ISet(1, R);
    p_SetExp(p2, 1, 1, R);  // t(1) * x(1)
    p_SetComp(p2, 7, R);    // gen(7)
    p_SetExp(p2, 2, 1, R);  // t(2) * x(2)
    p_SetComp(p2, 7, R);    // gen(7)
    p_Setm(p2, R);

    // Assign the generators to the ideal
    I->m[0] = p1;
    I->m[1] = p2;

    // Print the ideal to verify the components
    std::cout << "Ideal in R: " << std::endl;
    for (int i = 0; i < IDELEMS(I); i++) {
        std::cout << "I[" << i << "] = " << pString(I->m[i]) << std::endl;
    }

    // Clean up memory
    rDelete(R);

    return 0;
}
