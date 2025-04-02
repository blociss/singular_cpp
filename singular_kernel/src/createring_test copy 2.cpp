#include <singular/Singular/libsingular.h>
#include <flint/fmpq_mat.h>
#include <iostream>
#include <cstring>

// Assuming these are part of your project; adjust paths/names as needed
#include "feynman.h"
#include "polys/ext_fields/transext.h"
#include <singular/polys/flintconv.h>


// Computes a matrix A such that A * gram = PI
matrix liftIdeal(ideal PI, ideal gram, ring r) {
    std::cout << "[DEBUG] Entering liftIdeal" << std::endl;

    // Validate inputs
    if (!PI || !gram || !r) {
        std::cerr << "[ERROR] Invalid input: PI, gram, or ring is NULL" << std::endl;
        return NULL;
    }

    // Save and set the current ring
    ring savedRing = currRing;
    rChangeCurrRing(r);

    // Debug input ideals
    std::cout << "[DEBUG] PI = ";
    for (int i = 0; i < IDELEMS(PI); ++i) {
        if (PI->m[i]) {
            char* p_str = p_String(PI->m[i], r);
            std::cout << "[" << i << "]: " << p_str << " ";
            omFree(p_str);
        } else {
            std::cout << "[" << i << "]: 0 ";
        }
    }
    std::cout << std::endl;

    std::cout << "[DEBUG] gram = ";
    for (int i = 0; i < IDELEMS(gram); ++i) {
        if (gram->m[i]) {
            char* p_str = p_String(gram->m[i], r);
            std::cout << "[" << i << "]: " << p_str << " ";
            omFree(p_str);
        } else {
            std::cout << "[" << i << "]: 0 ";
        }
    }
    std::cout << std::endl;

    // Compute the lift using idLift
    ideal A_ideal = idLift(PI, gram, NULL, FALSE, FALSE, FALSE, NULL, GbDefault);
    if (!A_ideal) {
        std::cerr << "[ERROR] idLift failed to compute the lift" << std::endl;
        rChangeCurrRing(savedRing);
        return NULL;
    }

    // Debug the lift result
    std::cout << "[DEBUG] A_ideal = ";
    for (int i = 0; i < IDELEMS(A_ideal); ++i) {
        if (A_ideal->m[i]) {
            char* p_str = p_String(A_ideal->m[i], r);
            std::cout << "[" << i << "]: " << p_str << " ";
            omFree(p_str);
        } else {
            std::cout << "[" << i << "]: 0 ";
        }
    }
    std::cout << std::endl;

    // Convert the ideal to a matrix
    matrix A = id_Module2Matrix(A_ideal, r);
    if (!A) {
        std::cerr << "[ERROR] Failed to convert A_ideal to matrix" << std::endl;
        id_Delete(&A_ideal, r);
        rChangeCurrRing(savedRing);
        return NULL;
    }

    // Debug the resulting matrix
    std::cout << "[DEBUG] Matrix A = ";
    for (int i = 1; i <= A->nrows; ++i) {
        for (int j = 1; j <= A->ncols; ++j) {
            poly p = MATELEM(A, i, j);
            char* p_str = p ? p_String(p, r) : omStrDup("0");
            std::cout << p_str << " ";
            omFree(p_str);
        }
        std::cout << std::endl;
    }

    // Convert gram to a matrix for verification
    matrix gram_matrix = id_Module2Matrix(gram, r);
    if (!gram_matrix) {
        std::cerr << "[ERROR] Failed to convert gram to matrix" << std::endl;
        mp_Delete(&A, r);
        id_Delete(&A_ideal, r);
        rChangeCurrRing(savedRing);
        return NULL;
    }

    // Verify A * gram = PI
    matrix check = mp_Mult(A, gram_matrix, r);
    std::cout << "[DEBUG] A * gram = ";
    for (int i = 0; i < IDELEMS(check); ++i) {
        if (check->m[i]) {
            char* p_str = p_String(check->m[i], r);
            std::cout << "[" << i << "]: " << p_str << " ";
            omFree(p_str);
        } else {
            std::cout << "[" << i << "]: 0 ";
        }
    }
    std::cout << std::endl;

    // Cleanup
    id_Delete(&A_ideal, r);
    mp_Delete(&gram_matrix, r);
    mp_Delete(&check, r);

    rChangeCurrRing(savedRing);
    std::cout << "[DEBUG] Exiting liftIdeal" << std::endl;
    return A;
}
int main() {
    std::cout<<"[DEBUG] Entering main"<<std::endl;
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");

    // Example ring: QQ[p(1),p(2)]
    ring r = rDefault(0, 2, NULL, ringorder_dp);
    rChangeCurrRing(r);
std::cout<<"[DEBUG] Ring created: "<<rString(r)<<std::endl;
    // Example PI = <p(1), p(2)>
    ideal PI = idInit(2, 1);
    PI->m[0] = p_One(r); p_SetExp(PI->m[0], 1, 1, r); p_Setm(PI->m[0], r); // p(1)
    PI->m[1] = p_One(r); p_SetExp(PI->m[1], 2, 1, r); p_Setm(PI->m[1], r); // p(2)

    // Example gram = <p(1), p(1)+p(2)>
    ideal gram = idInit(2, 1);
    gram->m[0] = p_One(r); p_SetExp(gram->m[0], 1, 1, r); p_Setm(gram->m[0], r); // p(1)
    gram->m[1] = p_One(r); p_SetExp(gram->m[1], 1, 1, r); p_Add_q(gram->m[1], p_One(r), r); p_SetExp(pNext(gram->m[1]), 2, 1, r); p_Setm(gram->m[1], r); // p(1)+p(2)

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
    return 0;
}