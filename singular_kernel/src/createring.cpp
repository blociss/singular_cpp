    #include <iostream>
    #include <singular/Singular/libsingular.h>
    #include <singular/polys/ext_fields/transext.h>
    #include <singular/polys/monomials/ring.h>
    #include <singular/Singular/maps_ip.h>
    #include <singular/Singular/ipid.h>
    #include <singular/polys/simpleideals.h>
    #include <singular/coeffs/numbers.h> // For n_SetMap
    #include <singular/polys/prCopy.h>
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

    // Test different mapping approaches
    ring R1 = rCopy0(P); // Create a copy of P
    rComplete(R1);
    
    // Create a test polynomial in P
    poly p = p_ISet(1, P);
    p_SetExp(p, 1, 1, P); // Set x_1^1
    p_Setm(p, P);
    std::cout << "\nTest polynomial in P: " << p_String(p, P) << std::endl;
    
    // Try different mapping approaches
    std::cout << "\nTesting mapping approaches:\n";
    
    // 1. Using prCopyR
    rChangeCurrRing(R1);
    poly p1 = prCopyR(p, P, R1);
    std::cout << "1. prCopyR result: " << (p1 ? p_String(p1, R1) : "NULL") << std::endl;
    
    // 2. Using p_PermPoly
    int *perm = (int*)omAlloc0((rVar(P) + 1) * sizeof(int));
    for (int i = 0; i <= rVar(P); i++) perm[i] = i;
    poly p2 = p_PermPoly(p, perm, P, R1, NULL, NULL, 0, FALSE);
    std::cout << "2. p_PermPoly result: " << (p2 ? p_String(p2, R1) : "NULL") << std::endl;
    
    // 3. Using nSetMap and manual copying
    nMapFunc nMap = n_SetMap(P->cf, R1->cf);
    if (nMap) {
        poly p3 = p_Init(R1);
        number n = nMap(pGetCoeff(p), R1->cf);
        p_SetCoeff0(p3, n, R1);
        for (int i = 1; i <= rVar(P); i++) {
            p_SetExp(p3, i, p_GetExp(p, i, P), R1);
        }
        p_Setm(p3, R1);
        std::cout << "3. Manual mapping result: " << p_String(p3, R1) << std::endl;
        p_Delete(&p3, R1);
    } else {
        std::cout << "3. Manual mapping failed: nMap is NULL" << std::endl;
    }
    
    // Clean up
    omFreeSize((ADDRESS)perm, (rVar(P) + 1) * sizeof(int));
    p_Delete(&p, P);
    p_Delete(&p1, R1);
    p_Delete(&p2, R1);
    rDelete(R1);

    // Cleanup
    rKill(P);

    return 0;
}
