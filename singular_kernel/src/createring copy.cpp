    #include <iostream>
    #include <singular/Singular/libsingular.h>
    #include <singular/polys/ext_fields/transext.h>
    #include <singular/polys/monomials/ring.h>

    ring createRing(char **extNames, int extCount, char **varNames, int varCount, rRingOrder_t varOrdering) {
        // Create base coefficient field
        std::cout << "[DEBUG] Creating base coefficient field" << std::endl;
        coeffs cf = nInitChar(n_Q, NULL);
        if (!cf) return NULL;

        // Create extension field if needed
        ring extRing = NULL;
        if (extCount > 0) {
            // Create extension ring with default ordering
            extRing = rDefault(cf, extCount, extNames);
            if (!extRing) {
                nKillChar(cf);
                return NULL;
            }

            // Create extension coefficient field
            TransExtInfo extParam;
            extParam.r = extRing;
            coeffs extCf = nInitChar(n_transExt, &extParam);

            // Clean up and check
            rKill(extRing);
            nKillChar(cf);
            
            if (!extCf) return NULL;
            cf = extCf;
        }
    
        // Create variable names array
        char **vars = (char **)omAlloc0(varCount * sizeof(char *));
        if (!vars) {
            nKillChar(cf);
            return NULL;
        }

        // Copy variable names
        for (int i = 0; i < varCount; ++i) {
            vars[i] = omStrDup(varNames[i]);
            if (!vars[i]) {
                for (int j = 0; j < i; ++j) omFree(vars[j]);
                omFree(vars);
                nKillChar(cf);
                return NULL;
            }
        }

        // Create ring with specified ordering
        ring R = rDefault(cf, varCount, vars, varOrdering);
        // Clean up variable names
        for (int i = 0; i < varCount; ++i) omFree(vars[i]);
        omFree(vars);

        if (!R) {
            nKillChar(cf);
            return NULL;
        }

        // Initialize ring
        rComplete(R);

        return R;
    }
    int main() {
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    std::cout << "[DEBUG] Initializing Singular" << std::endl;

    // Create base coefficient field QQ (rationals)
    coeffs cf = nInitChar(n_Q, NULL);
    if (!cf) {
        std::cerr << "Failed to create coefficient field QQ\n";
        return 1;
    }
    std::cout << "[DEBUG] Created coefficient field QQ" << std::endl;

    // Define all variables explicitly
    char *vars[] = {
        (char*)"p(1)", (char*)"p(2)", (char*)"p(3)", (char*)"p(4)",
        (char*)"q(1)", (char*)"q(2)", (char*)"q(3)", (char*)"q(4)",
        (char*)"q(5)", (char*)"q(6)", (char*)"q(7)"
    };
    int varCount = 11;

    std::cout << "[DEBUG] Creating ring with " << varCount << " variables and dp ordering" << std::endl;

    // Create ring with dp ordering
    ring P = rDefault(cf, varCount, vars, ringorder_ip);
    if (!P) {
        std::cerr << "Failed to create ring\n";
        nKillChar(cf);
        return 1;
    }

    rComplete(P);

    // Set current ring
    rChangeCurrRing(P);

    // Print ring details (clean output)
    std::cout << "[DEBUG] Created ring: " << rString(P) << std::endl;
    std::cout << "Ring details:\n";
    rWrite(P);
    std::cout << "\nRing created with: " << rString(P) << "\n";

    // Cleanup
    rKill(P);

    return 0;
}
