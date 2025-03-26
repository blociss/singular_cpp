#include "feynman.h"
#include "singular_functions.hpp"
#include <iostream>
#include <singular/Singular/libsingular.h>
#include "polys/ext_fields/transext.h"
#include <singular/polys/monomials/ring.h>
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/polys/simpleideals.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/prCopy.h>
#include <singular/Singular/libsingular.h>
// Function to create a ring with variables as coefficients using Singular procedures
ring createRingWithParams() {
    // Initialize Singular
    // First create a ring with variables
    coeffs cf = nInitChar(n_Q, NULL); // Base field QQ
    char* varNames[] = {(char*)"p(1)", (char*)"p(2)", (char*)"p(3)", (char*)"p(4)"};
    ring R = rDefault(cf, 4, varNames);
    rChangeCurrRing(R);
    
    // Create an ideal to compute Groebner basis
    ideal I = idInit(2,1);
    poly p1 = p_ISet(1,R);
    p_SetExp(p1, 1, 2, R); // p(1)^2
    p_Setm(p1, R);
    I->m[0] = p1;
    
    poly p2 = p_ISet(1,R);
    p_SetExp(p2, 2, 2, R); // p(2)^2
    p_Setm(p2, R);
    I->m[1] = p2;
    
    // Convert ideal to lists for call_user_proc
    lists L = (lists)omAlloc0(sizeof(slists));
    L->Init(1);
    L->m[0].rtyp = IDEAL_CMD;
    L->m[0].data = I;
    
    // Load the standard library first
    std::string load_cmd = "LIB \"standard.lib\";";
    call_singular(load_cmd);
    
    // Call Singular's groebner function using call_user_proc
    std::string function_name = "groebner";
    std::string library = "standard.lib";
    auto result = call_user_proc(function_name, library, 0, L);
    
    if (result.first != 0) {
        // Error occurred
        return NULL;
    }
    
    // Result is now in result.second
    ideal J = (ideal)(result.second->m[0].data);
    
    // Clean up
    omFree(L);
    
    return R;
}

// Example usage
int main() {
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    ring R = createRingWithParams();
    if (R) {
        std::cout << "Ring created: " << rString(R) << std::endl;
        rDelete(R); // Clean up the ring when done
    }
    return 0;
}