#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>

int main() {
    // Initialize Singular library
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Define the coefficient field (ℚ)
    ring r;

    // Define coefficient variables t(1), t(2)
    const int numCoeffVars = 2;  // t(1), t(2)
    const int numVars = 9;       // z(1) to z(9)

    // Dynamically create the names for the coefficient variables t(1), t(2)
    char *coeffVars[numCoeffVars];
    for (int i = 0; i < numCoeffVars; ++i) {
        coeffVars[i] = (char*)omAlloc(5 * sizeof(char)); // "t(i)" where i = 1,2
        sprintf(coeffVars[i], "t(%d)", i + 1);
    }

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
    block0[0] = 1;  // First block
    int* block1 = (int*)omAlloc0(3 * sizeof(int));
    block1[0] = 4;  // Second block

    // Define the ring using rDefault function
    r = rDefault(0, numVars + numCoeffVars, coeffVars, numCoeffVars, order, block0, block1);  // 0 means coefficient field is ℚ

    // Change the current ring
    rChangeCurrRing(r);

    // Initialize the ideal
    int initialSize = 36;  // Initial size of the ideal (9 choose 2 = 36)
    ideal I = idInit(initialSize, 1);
    if (!I) {
        std::cerr << "Failed to initialize ideal" << std::endl;
        return 1;
    }
    std::cout << "Ideal initialized with " << initialSize << " generators." << std::endl;

    // Add terms to the ideal using a loop
    int index = 0;
    for (int i = 0; i < numVars; i++) {
        for (int j = i + 1; j < numVars; j++) {
            // Create the term (t(1) + t(2)) * z(i) * z(j)
            poly p1 = p_ISet(1, r); // Start with constant polynomial 1
            p_SetExp(p1, 1, 1, r); // Set exponent of t(1) to 1
            p_SetExp(p1, 2, 1, r); // Set exponent of t(2) to 1
            
            poly p2 = p_ISet(1, r); // Start with constant polynomial 1
            p_SetExp(p2, i + 3, 1, r); // Set exponent of z(i) to 1
            p_SetExp(p2, j + 3, 1, r); // Set exponent of z(j) to 1

            poly p = p_Mult_q(p1, p2, r); // Multiply p1 and p2
           // p_Setm(p, r);
           // std::cout << "p:=" << pString(p) << std::endl;

            p_SetComp(p,  i+1, currRing);  // Assign the component index
            //  std::cout << "p_SetComp:=" <<p_SetComp(t0, i + 1, currRing)<< std::endl;
             
            p_SetmComp(p, currRing);       // t=m[j,i]*gen(i)
            std::cout << "t" <<i<<"= "<< pString(p) << std::endl;
            //  std::cout << "p_SetmComp:=" <<pString(t0)<< std::endl;
           // t = pCopy(t0); 
            // Add the polynomial to the ideal
            I->m[index++] = p;
        }
    }
	idPrint(I);
    // Output the ideal's generators
    std::cout << "Ideal generators:" << std::endl;
    for (int i = 0; i < initialSize; ++i) {
        std::cout << "I->m[" << i << "]: ";
        char* polyStr = p_String(I->m[i], r);
        if (polyStr) {
            std::cout << polyStr << std::endl;
            omFree(polyStr); // Free memory
        } else {
            std::cerr << "Failed to convert polynomial to string" << std::endl;
        }
    }
// Compute the Gröbner basis
    ideal G = kStd(I, NULL, testHomog, NULL);

    std::cout << "Gröbner basis computed." << std::endl;

    // Check if Gröbner basis was computed
    if (!G) {
        std::cerr << "Failed to compute Gröbner basis" << std::endl;
        return 1;
    }

    // Print the size of the Gröbner basis
    std::cout << "Gröbner basis has " << IDELEMS(G) << " elements:" << std::endl;

    // Print the Gröbner basis
    for (int i = 0; i < IDELEMS(G); i++) {
        char* polyStr = p_String(G->m[i], r);
        if (polyStr) {
            std::cout << "G[" << i << "]: " << polyStr << std::endl;
            omFree(polyStr); // Free memory
        } else {
            std::cerr << "Failed to convert polynomial to string" << std::endl;
        }
    } 
    int tt= IDELEMS(G);
    int p= IDELEMS(I);
    // Free allocated memory
    for (int i = 0; i < numCoeffVars; ++i) {
        omFree(coeffVars[i]);
    }
    for (int i = 0; i < numVars; ++i) {
        omFree(vars[i]);
    }
    omFree(order);
    omFree(block0);
    omFree(block1);

    // Clean up the ideal
    id_Delete(&I, r);

    return 0;
}