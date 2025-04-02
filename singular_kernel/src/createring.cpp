#include <iostream>
#include <vector>
#include <singular/Singular/libsingular.h>
#include <singular/polys/ext_fields/transext.h>
#include <singular/polys/monomials/ring.h>

// Function to create a ring with extension field and variables
ring createExtendedPolyRing(int ct, int anzq) {
    std::cout << "[DEBUG] Starting createExtendedPolyRing" << std::endl;
    // Step 1: Create base coefficient field QQ
    coeffs baseCoeff = nInitChar(n_Q, NULL);
    if (!baseCoeff) {
        std::cerr << "[ERROR] Failed to create base coefficient field QQ" << std::endl;
        return NULL;
    }
    std::cout << "[DEBUG] Created base coefficient field QQ" << std::endl;

    // Step 2: Create coefficient ring QQ(p(1), ..., p(ct)) with dp ordering
    std::vector<char*> varsCoeff;
    for (int i = 1; i <= ct; ++i) {
        std::string name = "p(" + std::to_string(i) + ")";
        varsCoeff.push_back(omStrDup(name.c_str()));
        std::cout << "[DEBUG] Added parameter: " << name << std::endl;
    }

    ring coeffRing = rDefault(baseCoeff, ct, varsCoeff.data(), ringorder_dp);
    if (!coeffRing) {
        std::cerr << "[ERROR] Failed to create coefficient ring" << std::endl;
        nKillChar(baseCoeff);
        for (char* var : varsCoeff) omFree(var);
        return NULL;
    }
    rComplete(coeffRing);
    std::cout << "[DEBUG] Created coefficient ring: " << rString(coeffRing) << std::endl;

    // Step 3: Create transcendental extension field QQ(p(1), ..., p(ct))
    TransExtInfo param = {coeffRing};
    coeffs extCoeff = nInitChar(n_transExt, &param);
    if (!extCoeff) {
        std::cerr << "[ERROR] Failed to create extension coefficient field" << std::endl;
        rDelete(coeffRing);
        nKillChar(baseCoeff);
        for (char* var : varsCoeff) omFree(var);
        return NULL;
    }
    std::cout << "[DEBUG] Extension coefficient field created: QQ(p(1)..p(" << ct << "))" << std::endl;

    // Clean up base coefficient field (no longer needed after extension)
    nKillChar(baseCoeff);

    // Step 4: Create polynomial ring R: QQ(p(1), ..., p(ct))[q(1), ..., q(anzq)], ip ordering
    std::vector<char*> varsR;
    std::cout << "[DEBUG] Creating variables for ring R:" << std::endl;
    for (int i = 1; i <= anzq; ++i) {
        std::string name = "q(" + std::to_string(i) + ")";
        varsR.push_back(omStrDup(name.c_str()));
        std::cout << "[DEBUG] Added variable: " << name << std::endl;
    }

    ring R = rDefault(extCoeff, anzq, varsR.data(), ringorder_ip);
    if (!R) {
        std::cerr << "[ERROR] Failed to create polynomial ring R" << std::endl;
        nKillChar(extCoeff);
        rDelete(coeffRing);
        for (char* var : varsCoeff) omFree(var);
        for (char* var : varsR) omFree(var);
        return NULL;
    }
    rComplete(R);

    // Clean up variable names
    for (char* var : varsCoeff) omFree(var);
    for (char* var : varsR) omFree(var);

    std::cout << "[DEBUG] Created polynomial ring R: " << rString(R) << std::endl;
    rWrite(R);
    std::cout << std::endl;

    return R;
}

int main() {
    // Initialize Singular library
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    std::cout << "[DEBUG] Initializing Singular" << std::endl;

    // Create ring with 4 parameters and 7 variables
    int ct = 4;    // Number of parameters p(1), ..., p(4)
    int anzq = 7;  // Number of variables q(1), ..., q(7)
    ring R = createExtendedPolyRing(ct, anzq);
    if (!R) {
        std::cerr << "[ERROR] Failed to create ring" << std::endl;
        return 1;
    }

    // Set as current ring
    rChangeCurrRing(R);

    // Clean up
    rDelete(R);
    std::cout << "[DEBUG] Ring deleted, program complete" << std::endl;

    return 0;
}