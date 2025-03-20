#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"


// Function to create a custom ring
ring my_ring(const char** var_names, int num_vars, int num_coeffs, const int* blocks) {

    // Create the coefficient field (ℚ in this case)
    coeffs cf = nInitChar(n_Q, NULL); // ℚ (rational numbers)

    // Create the dynamic variable names for z(1), z(2), ..., z(num_vars)
    char** vars = (char**)omAlloc(num_vars * sizeof(char*));
    for (int i = 0; i < num_vars; ++i) {
        vars[i] = (char*)omAlloc(5 * sizeof(char)); // "z(i)" where i = 1,2,...,9
        sprintf(vars[i], "z(%d)", i + 1);
    }

    
        // Define the orders (dp(9), C)
    rRingOrder_t* order = (rRingOrder_t*)omAlloc0(3 * sizeof(rRingOrder_t));
    order[0] = ringorder_dp;  // dp(9)
    order[1] = ringorder_c;   // C (for the coefficient field)

    // Define the blocks of the ring
    int* block0 = (int*)omAlloc(3 * sizeof(int));
    block0[0] = blocks[0];  // Size of the first block
    block0[1] = 1;          // Start index of the first block
    block0[2] = blocks[0];  // End index of the first block

    int* block1 = (int*)omAlloc(3 * sizeof(int));
    block1[0] = blocks[1];  // Size of the second block
    block1[1] = blocks[0] + 1;  // Start index of the second block
    block1[2] = blocks[0] + blocks[1];  // End index of the second block

    // Create the ring R using rDefault function
    ring R = rDefault(cf, num_vars, vars, 2, order, block0, block1);

    // Free memory
    for (int i = 0; i < num_vars; ++i) {
        omFree(vars[i]);
    }
    omFree(vars);
    omFree(order);
    omFree(block0);
    omFree(block1);

    return R;
}

int main() {
        siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Define the names of the coefficient variables (can be changed)
    const char* var_names[] = {"t(1)", "t(2)", "D"};

    // Define the number of coefficient variables (can be changed)
    int num_coeffs = 3;

    // Define the number of variables in the ring (can be changed)
    int num_vars = 9;

    // Define the blocks (can be changed)
    int blocks[] = {1, 9};

    // Create the ring R using the my_ring function
    ring R = my_ring(var_names, num_vars, num_coeffs, blocks);

    // Print a confirmation that the ring has been created
    std::cout << "Ring created with " << num_vars << " variables and " << num_coeffs << " coefficient fields." << std::endl;
    rChangeCurrRing(R);

    std::cout << "Current ring: " << rString(currRing) << std::endl;
/* 
    // Create the first ideal I1
    ideal I1 = idInit(2, 1);
    poly p1 = p_ISet(1, R);  // Start with constant polynomial 1
    p_SetExp(p1, 1, 1, R);   // Set exponent of t(1) to 1
    p_Setm(p1, R);
    I1->m[0] = p1;

    poly p2 = p_ISet(1, R);  // Start with constant polynomial 1
    p_SetExp(p2, 2, 1, R);   // Set exponent of t(2) to 1
    p_Setm(p2, R);
    I1->m[1] = p2;

    // Create the second ideal I2
    ideal I2 = idInit(2, 1);
    poly p3 = p_ISet(1, R);  // Start with constant polynomial 1
    p_SetExp(p3, 1, 1, R);   // Set exponent of t(1) to 1
    p_SetExp(p3, 2, 1, R);   // Set exponent of t(2) to 1
    p_Setm(p3, R);
    I2->m[0] = p3;

    poly p4 = p_ISet(1, R);  // Start with constant polynomial 1
    p_SetExp(p4, 3, 1, R);   // Set exponent of D to 1
    p_Setm(p4, R);
    I2->m[1] = p4;

    // Compute the intersection of I1 and I2
    ideal Intersection = idSect(I1, I2);

    // Print the intersection
    std::cout << "Intersection of I1 and I2:" << std::endl;
    for (int i = 0; i < IDELEMS(Intersection); ++i) {
        char* polyStr = p_String(Intersection->m[i], R);
        if (polyStr) {
            std::cout << "Intersection[" << i << "]: " << polyStr << std::endl;
            omFree(polyStr);  // Free memory
        } else {
            std::cerr << "Failed to convert polynomial to string" << std::endl;
        }
    } 

    // Clean up memory for the ideals and ring
    id_Delete(&I1, R);
    id_Delete(&I2, R);
    id_Delete(&Intersection, R);*/
    rDelete(R);

    return 0;
}