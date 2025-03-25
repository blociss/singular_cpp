#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// Singular includes
#include <singular/Singular/libsingular.h>
#include <singular/polys/ext_fields/transext.h>
#include <singular/polys/monomials/ring.h>
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/polys/simpleideals.h>
#include <singular/coeffs/numbers.h>
#include <singular/polys/prCopy.h>

// Helper function to create a polynomial ring
ring createRing(char **extNames, int extCount, char **varNames, int varCount, rRingOrder_t varOrdering) {
    // Create base coefficient field (rational numbers)
    coeffs cf = nInitChar(n_Q, NULL);
    if (!cf) return NULL;

    // Create extension field if extension names are provided
    ring extRing = NULL;
    if (extCount > 0) {
        extRing = rDefault(cf, extCount, extNames);
        if (!extRing) {
            nKillChar(cf);
            return NULL;
        }

        TransExtInfo extParam;
        extParam.r = extRing;
        coeffs extCf = nInitChar(n_transExt, &extParam);

        // Clean up intermediate structures
        rKill(extRing);
        nKillChar(cf);
        
        if (!extCf) return NULL;
        cf = extCf;
    }
    
    // Allocate and copy variable names
    char **vars = (char **)omAlloc0(varCount * sizeof(char *));
    if (!vars) {
        nKillChar(cf);
        return NULL;
    }

    for (int i = 0; i < varCount; ++i) {
        vars[i] = omStrDup(varNames[i]);
        if (!vars[i]) {
            for (int j = 0; j < i; ++j) omFree(vars[j]);
            omFree(vars);
            nKillChar(cf);
            return NULL;
        }
    }
    
    // Create the ring with the specified ordering
    ring R = rDefault(cf, varCount, vars, varOrdering);

    // Clean up variable names
    for (int i = 0; i < varCount; ++i) omFree(vars[i]);
    omFree(vars);

    if (!R) {
        nKillChar(cf);
        return NULL;
    }

    // Finalize ring initialization
    rComplete(R);

    return R;
}

// Helper function to print an ideal
void printIdeal(ideal I, ring r, const char* name) {
    rChangeCurrRing(r);
    std::cout << name << ":\n";
    for (int i = 0; i < IDELEMS(I); i++) {
        if (I->m[i] != NULL) {
            char* s = p_String(I->m[i], r);
            std::cout << "  " << s << std::endl;
            omFree(s);
        } else {
            std::cout << "  NULL polynomial" << std::endl;
        }
    }
    std::cout << std::endl;
}

// Main function
int main() {
    // Initialize Singular library (update path to libSingular.so)
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");

    // Step 1: Create ring RP with variables p(1), p(2), p(3), p(4), q(1), ..., q(7)
    char* rp_vars[] = {(char*)"p(1)", (char*)"p(2)", (char*)"p(3)", (char*)"p(4)", (char*)"q(1)", (char*)"q(2)", (char*)"q(3)", (char*)"q(4)", (char*)"q(5)", (char*)"q(6)", (char*)"q(7)"};
    ring RP = createRing(NULL, 0, rp_vars, 11, ringorder_dp);  // Degree reverse lexicographical ordering
    if (!RP) {
        std::cerr << "Failed to create RP" << std::endl;
        return 1;
    }
    std::cout << "Ring RP created: " << rString(RP) << std::endl;

    // Step 2: Create ring R with extension field p(1), p(2), p(3), p(4) and variables q(1), ..., q(7)
    char* ext_names[] = {(char*)"p(1)", (char*)"p(2)", (char*)"p(3)", (char*)"p(4)"};
    char* r_vars[] = {(char*)"q(1)", (char*)"q(2)", (char*)"q(3)", (char*)"q(4)", (char*)"q(5)", (char*)"q(6)", (char*)"q(7)"};
    ring R = createRing(ext_names, 4, r_vars, 7, ringorder_dp);
    if (!R) {
        std::cerr << "Failed to create R" << std::endl;
        rDelete(RP);
        return 1;
    }
    std::cout << "Ring R created: " << rString(R) << std::endl;

    // Step 3: Define the balancing ideal in ring R
   rChangeCurrRing(R);
ideal I = idInit(7, 1);  // Initialize ideal with 7 elements
const char* polys[7] = {
    "q(4)+q(1)+p(1)",
    "q(5)-q(1)+p(2)",
    "q(7)-q(5)+q(2)",
    "-q(7)+q(3)+p(4)",
    "q(6)-q(3)+p(3)",
    "-q(6)-q(4)-q(2)",
    "p(1)+p(2)+p(3)+p(4)"
};
for (int i = 0; i < 7; i++) {
    I->m[i] = p_Init(R);
    p_Read(polys[i], I->m[i], R);
}
std::cout << "Balancing ideal in RP:\n";
for(int i=0; i<IDELEMS(I); i++){
    std::cout<<p_String(I->m[i], RP)<<std::endl;
}
printIdeal(I, R, "Balancing ideal in R");

    // Step 4: Map the ideal from R to RP
    rChangeCurrRing(RP);
    ideal I_mapped = idInit(7, 1);
    for (int i = 0; i < 7; i++) {
        if (I->m[i] != NULL) {
            char* s = p_String(I->m[i], R);
            std::string str(s);
            // Adjust notation: "(p(i))" in R becomes "p(i)" in RP
            for (int j = 1; j <= 4; j++) {
                std::string old_pat = "(p(" + std::to_string(j) + "))";
                std::string new_pat = "p(" + std::to_string(j) + ")";
                size_t pos = 0;
                while ((pos = str.find(old_pat, pos)) != std::string::npos) {
                    str.replace(pos, old_pat.length(), new_pat);
                    pos += new_pat.length();
                }
            }
            I_mapped->m[i] = p_Init(RP);
            p_Read(str.c_str(), I_mapped->m[i], RP);
            omFree(s);
        } else {
            I_mapped->m[i] = NULL;
        }
    }
    printIdeal(I_mapped, RP, "Mapped ideal in RP");

    // Step 5: Compute the standard basis in RP
    ideal I_std = kStd(I_mapped, NULL, (tHomog)FALSE, NULL, 0, 0, TRUE);
    printIdeal(I_std, RP, "Standard basis in RP");

    // Step 6: Clean up memory
    id_Delete(&I, R);
    id_Delete(&I_mapped, RP);
    id_Delete(&I_std, RP);
    rDelete(R);
    rDelete(RP);

    return 0;
}