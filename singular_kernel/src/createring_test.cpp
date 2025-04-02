#include <singular/Singular/libsingular.h>
#include <flint/fmpq_mat.h>
#include <iostream>
#include <cstring>
#include <vector>
#include "feynman.h"
#include "polys/ext_fields/transext.h"
#include <singular/polys/flintconv.h>

// Computes a matrix A such that A * gram = PI dynamically for general ideals
matrix liftIdeal(ideal PI, ideal gram, ring r) {
    std::cout << "[DEBUG] Entering general liftIdeal" << std::endl;

    if (!PI || !gram || !r) {
        std::cerr << "[ERROR] Invalid input: PI, gram, or ring is NULL" << std::endl;
        return NULL;
    }

    ring savedRing = currRing;
    rChangeCurrRing(r);

    std::cout << "[DEBUG] PI = ";
    printIdeal(PI);
    std::cout << "[DEBUG] gram = ";
    printIdeal(gram);

    int pi_rows = IDELEMS(PI);    // Rows of A (PI generators)
    int gram_cols = IDELEMS(gram); // Columns of A (gram generators)
    int nvars = rVar(r);          // Number of variables in the ring

    // Create matrix A
    matrix A = mpNew(pi_rows, gram_cols);
    if (!A) {
        std::cerr << "[ERROR] Failed to allocate matrix A" << std::endl;
        rChangeCurrRing(savedRing);
        return NULL;
    }

    // For each PI generator, express it as a linear combination of gram generators
    for (int i = 0; i < pi_rows; ++i) {
        poly pi_gen = PI->m[i];
        if (!pi_gen) continue;

        // Try to match pi_gen with linear combinations of gram generators
        for (int j = 0; j < gram_cols; ++j) {
            poly gram_gen = gram->m[j];
            if (!gram_gen) continue;

            // Compute the coefficient by "dividing" pi_gen by gram_gen
            poly quotient = p_Divide(pi_gen, gram_gen, r);
            if (quotient && p_IsConstant(quotient, r)) {
                // If division yields a constant, use it as the coefficient
                number coeff = p_GetCoeff(quotient, r);
                MATELEM(A, i + 1, j + 1) = p_NSet(n_Copy(coeff, r->cf), r);
                p_Delete(&quotient, r);
            } else {
                // If not directly divisible, try subtraction and solve
                poly diff = p_Sub(p_Copy(pi_gen, r), p_Copy(gram_gen, r), r);
                if (diff) {
                    MATELEM(A, i + 1, j + 1) = p_ISet(1, r); // Exact match
                } else {
                    // Check if pi_gen can be expressed via other gram terms
                    bool matched = false;
                    for (int k = 0; k < gram_cols; ++k) {
                        if (k == j) continue;
                        poly other_gram = gram->m[k];
                        if (!other_gram) continue;
                        poly temp = p_Sub(p_Copy(diff, r), p_Copy(other_gram, r), r);
                        if (temp) {
                            MATELEM(A, i + 1, j + 1) = p_ISet(1, r);
                            MATELEM(A, i + 1, k + 1) = p_ISet(-1, r);
                            matched = true;
                            p_Delete(&temp, r);
                            break;
                        }
                        p_Delete(&temp, r);
                    }
                    if (!matched && j == 0) {
                        // Default to 0 if no match found (first column)
                        MATELEM(A, i + 1, j + 1) = p_ISet(0, r);
                    }
                    p_Delete(&diff, r);
                }
            }
        }
    }

    std::cout << "[DEBUG] Matrix A = ";
    printMatrix(A);

    // Convert gram to matrix for verification
    matrix gram_matrix = id_Module2Matrix(gram, r);
    if (!gram_matrix) {
        std::cerr << "[ERROR] Failed to convert gram to matrix" << std::endl;
        mp_Delete(&A, r);
        rChangeCurrRing(savedRing);
        return NULL;
    }

    std::cout << "[DEBUG] gram_matrix = ";
    printMatrix(gram_matrix);

    // Verify A * gram = PI
    matrix check = mp_Mult(A, gram_matrix, r);
    if (!check) {
        std::cerr << "[ERROR] mp_Mult failed" << std::endl;
    } else {
        std::cout << "[DEBUG] A * gram = ";
        printMatrix(check);
    }

    mp_Delete(&gram_matrix, r);
    if (check) mp_Delete(&check, r);

    rChangeCurrRing(savedRing);
    std::cout << "[DEBUG] Exiting general liftIdeal" << std::endl;
    return A;
}

int main() {
    std::cout << "[DEBUG] Entering main" << std::endl;
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");

    coeffs baseCoeff = nInitChar(n_Q, NULL);
    if (!baseCoeff) {
        std::cerr << "[ERROR] Failed to create base coefficient field QQ" << std::endl;
        return 1;
    }

    int ct = 4;
    std::vector<char*> varsCoeff;
    for (int i = 1; i <= ct; ++i) {
        std::string name = "p(" + std::to_string(i) + ")";
        varsCoeff.push_back(omStrDup(name.c_str()));
    }

    ring coeffRing = rDefault(baseCoeff, ct, varsCoeff.data(), ringorder_dp);
    if (!coeffRing) {
        nKillChar(baseCoeff);
        for (char* var : varsCoeff) omFree(var);
        return 1;
    }
    rComplete(coeffRing);

    TransExtInfo param = {coeffRing};
    coeffs extCoeff = nInitChar(n_transExt, &param); // Corrected from ¶m to &param
    if (!extCoeff) {
        rDelete(coeffRing);
        nKillChar(baseCoeff);
        for (char* var : varsCoeff) omFree(var);
        return 1;
    }
    nKillChar(baseCoeff);

    int anzq = 7;
    std::vector<char*> varsR;
    for (int i = 1; i <= anzq; ++i) {
        std::string name = "q(" + std::to_string(i) + ")";
        varsR.push_back(omStrDup(name.c_str()));
    }

    ring r = rDefault(extCoeff, anzq, varsR.data(), ringorder_dp);
    if (!r) {
        nKillChar(extCoeff);
        rDelete(coeffRing);
        for (char* var : varsCoeff) omFree(var);
        for (char* var : varsR) omFree(var);
        return 1;
    }
    rComplete(r);
    rChangeCurrRing(r);

    std::cout << "[DEBUG] Ring created: " << rString(r) << std::endl;

    // Example PI and gram (more general case)
    ideal PI = idInit(2, 1);
    PI->m[0] = p_One(r); p_SetExp(PI->m[0], 1, 1, r); p_Setm(PI->m[0], r); // q(1)
    PI->m[1] = p_One(r); p_SetExp(PI->m[1], 2, 1, r); p_Setm(PI->m[1], r); // q(2)

    ideal gram = idInit(2, 1);
    gram->m[0] = p_One(r); p_SetExp(gram->m[0], 1, 1, r); p_Setm(gram->m[0], r); // q(1)
    poly q1 = p_One(r); p_SetExp(q1, 1, 1, r); p_Setm(q1, r);
    poly q2 = p_One(r); p_SetExp(q2, 2, 1, r); p_Setm(q2, r);
    gram->m[1] = p_Add_q(q1, q2, r); // q(1)+q(2)

    std::cout << "[DEBUG] PI: " << std::endl;
    printIdeal(PI);
    std::cout << "[DEBUG] gram: " << std::endl;
    printIdeal(gram);

    matrix A = liftIdeal(PI, gram, r);

    if (A) {
        std::cout << "Lift matrix computed successfully" << std::endl;
        mp_Delete(&A, r);
    } else {
        std::cout << "Failed to compute lift matrix" << std::endl;
    }

    id_Delete(&PI, r);
    id_Delete(&gram, r);
    rDelete(r);
    for (char* var : varsCoeff) omFree(var);
    for (char* var : varsR) omFree(var);

    return 0;
}