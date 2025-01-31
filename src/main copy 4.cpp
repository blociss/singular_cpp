#include <singular/Singular/libsingular.h>
#include <iostream>
#include <vector>

// Custom function to create an ideal and compute its Grobner basis
ideal create_and_compute_grobner_basis(ring R, int num_vars) {
    // Create and initialize polynomials p1, p2, ..., pn dynamically
    ideal L = idInit(num_vars, 1);  // num_vars generators for the ideal
    
    for (int i = 0; i < num_vars; i++) {
        poly p = p_ISet(1, R);  // Set polynomial with coefficient 1
        pSetExp(p, i + 1, 1);   // Set exponent for the i+1-th variable
        pSetm(p);               // Update monomial
        L->m[i] = pCopy(p);     // Copy the polynomial into the ideal
    }

    // Compute Grobner basis using kStd
    ideal variety_ideal_std = kStd(L, NULL, testHomog, NULL);

    // Clean up the initial ideal
    idDelete(&L);

    return variety_ideal_std;
}

// Function to print an ideal's generators
void print_ideal_generators(ideal I, const char* ideal_name) {
    if (!I) {
        std::cout << "Ideal is NULL" << std::endl;
        return;
    }

    std::cout << "Generators of " << ideal_name << " (" 
              << IDELEMS(I) << " elements):\n";
    for (int i = 0; i < IDELEMS(I); i++) {
        if (I->m[i]) {
            std::cout << "Generator " << i << ": ";
            pWrite(I->m[i]);
            std::cout << std::endl;
        }
    }
}

int main() {
    // Initialize Singular
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");
    
    // Number of variables
    const int NUM_VARS = 52;

    // Construct the ring Z/32003[x1, x2, ..., x52]
    char **var_names = (char**)omalloc(NUM_VARS * sizeof(char*));
    for (int i = 0; i < NUM_VARS; i++) {
        var_names[i] = omStrDup(("x" + std::to_string(i+1)).c_str());
    }
    
    // Create the ring
    ring R = rDefault(32003, NUM_VARS, var_names);
    rChangeCurrRing(R);

    // Compute Grobner basis using our custom function
    ideal variety_ideal_std = create_and_compute_grobner_basis(R, NUM_VARS);

    // Print the Grobner basis
    print_ideal_generators(variety_ideal_std, "variety_ideal_std");

    // Clean up
    idDelete(&variety_ideal_std);
    rKill(R);

    // Free variable names
    for (int i = 0; i < NUM_VARS; i++) {
        omFree(var_names[i]);
    }
    omFree(var_names);

    return 0;
}