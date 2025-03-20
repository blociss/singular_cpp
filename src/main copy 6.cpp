#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>

int main() {
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Define variable names dynamically
    const int numVars = 10;
    char** vars = new char*[numVars];
    const char* varNames[] = { "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10" };

    for (int i = 0; i < numVars; i++) {
        vars[i] = strdup(varNames[i]);  // Allocate writable memory for variable names
    }

    // Create a polynomial ring over Q with 10 variables and degree reverse lex order (dp)
    ring r = rDefault(0, numVars, vars);
    rChangeCurrRing(r);

    // Initialize an ideal with the correct number of generators
    int idealSize = numVars * (numVars - 1) / 2 + numVars; // Total number of generators
    ideal I = idInit(idealSize, 1);
    if (!I) {
        std::cerr << "Failed to initialize ideal" << std::endl;
        return 1;
    }

    std::cout << "Ideal initialized with " << idealSize << " generators." << std::endl;

    // Add terms to the ideal using a loop
    int index = 0;
    for (int i = 0; i < numVars; i++) {
        for (int j = i + 1; j < numVars; j++) {
            // Create a polynomial for x_i * x_j using p_ISet
            poly p = p_ISet(1, r); // Start with constant polynomial 1
            if (!p) {
                std::cerr << "Failed to create initial polynomial" << std::endl;
                return 1;
            }

            // Set the exponents for x_i and x_j
            p_SetExp(p, i + 1, 1, r); // Set exponent of x_(i+1) to 1
            p_SetExp(p, j + 1, 1, r); // Set exponent of x_(j+1) to 1

            // Update the monomial representation
            p_Setm(p, r);

            // Add this polynomial to the ideal
            if (index < idealSize) {
                I->m[index++] = p;
            } else {
                std::cerr << "Index exceeds ideal size" << std::endl;
                return 1;
            }
        }
    }

    // Add cubic terms to the ideal
    for (int i = 0; i < numVars; i++) {
        poly p = p_ISet(1, r); // Start with constant polynomial 1
        if (!p) {
            std::cerr << "Failed to create initial polynomial" << std::endl;
            return 1;
        }

        // Set the exponent for x_i^3
        p_SetExp(p, i + 1, 3, r); // Set exponent of x_(i+1) to 3

        // Update the monomial representation
        p_Setm(p, r);

        // Add this polynomial to the ideal
        if (index < idealSize) {
            I->m[index++] = p;
        } else {
            std::cerr << "Index exceeds ideal size" << std::endl;
            return 1;
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
// Prepare the output token
    lists output = (lists)omAlloc0Bin(slists_bin);
    output->Init(4);

    lists t = (lists)omAlloc0Bin(slists_bin);
    t->Init(2);
    t->m[0].rtyp = STRING_CMD; t->m[0].data = strdup("generators");
    t->m[1].rtyp = STRING_CMD; t->m[1].data = strdup("module_std");
    output->m[1].rtyp = LIST_CMD; output->m[1].data = t;
    output->m[0].rtyp = RING_CMD; output->m[0].data = currRing;
    output->m[2].rtyp = RING_CMD; output->m[2].data = currRing;

    t = (lists)omAlloc0Bin(slists_bin);
    t->Init(tt); // Use the size of Li to initialize t
    for (int i = 0; i < tt; i++) {
        t->m[i].rtyp = POLY_CMD; t->m[i].data = pCopy((poly)G->m[i]);
    }
    output->m[3].rtyp = LIST_CMD; output->m[3].data = t;

    // Cleanup
    id_Delete(&I, currRing); // Cleanup L if no longer needed

    return {p};
}
