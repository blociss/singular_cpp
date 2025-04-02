#include <singular/Singular/libsingular.h>
#include <flint/fmpq_mat.h>
#include <iostream>
#include <cstring>

// Assuming these are part of your project; adjust paths/names as needed
#include "feynman.h"
#include "polys/ext_fields/transext.h"
#include <singular/polys/flintconv.h>

// Print an fmpq_mat_t matrix for debugging
void print_fmpq_mat(const char* label, const fmpq_mat_t mat) {
    std::cout << label << "\n-------------\n";
    for (slong i = 0; i < fmpq_mat_nrows(mat); i++) {
        for (slong j = 0; j < fmpq_mat_ncols(mat); j++) {
            fmpq_print(fmpq_mat_entry(mat, i, j));
            std::cout << "  ";
        }
        std::cout << "\n";
    }
    std::cout << "-------------\n";
}

// Convert FLINT fmpq_mat_t to Singular matrix
matrix convFlintFmpqMatSingM(fmpq_mat_t mat, const ring R) {
    matrix M = mpNew(fmpq_mat_nrows(mat), fmpq_mat_ncols(mat));
    for (slong i = 0; i < fmpq_mat_nrows(mat); i++) {
        for (slong j = 0; j < fmpq_mat_ncols(mat); j++) {
            number n = convFlintNSingN(fmpq_mat_entry(mat, i, j), R->cf);
            MATELEM(M, i + 1, j + 1) = p_NSet(n, R); // Singular indices are 1-based
        }
    }
    return M;
}


int main() {
    // Initialize Singular
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    std::cout << "Singular initialized.\n";

    // Initialize FLINT matrix (2x2 rational matrix)
    fmpq_mat_t mat;
    fmpq_mat_init(mat, 2, 2);
    fmpq_set_si(fmpq_mat_entry(mat, 0, 0), 1, 2);  // 1/2
    fmpq_set_si(fmpq_mat_entry(mat, 0, 1), 3, 1);  // 3
    fmpq_set_si(fmpq_mat_entry(mat, 1, 0), -1, 4); // -1/4
    fmpq_set_si(fmpq_mat_entry(mat, 1, 1), 5, 3);  // 5/3

    // Print the FLINT matrix
    print_fmpq_mat("FLINT Matrix", mat);

    // Create Singular ring using createRing
    char* var_names[] = {(char*)"x", (char*)"y"};
    ring R = createRing(NULL, 0, var_names, 2, ringorder_dp); // No extension fields, 2 vars, dp ordering
    if (R == NULL) {
        std::cout << "Error creating Singular ring.\n";
        fmpq_mat_clear(mat);
        return 1;
    }
    std::cout << "[DEBUG] Ring created: " << rString(R) << std::endl;
    rChangeCurrRing(R);
    std::cout << "Singular ring created: Q[x,y] with dp ordering.\n";

    // Convert FLINT matrix to Singular matrix
    matrix M = convFlintFmpqMatSingM(mat, R);
    std::cout << "Singular matrix 'M' created from FLINT data.\n";

    // Print the Singular matrix
    std::cout << "Singular matrix output:";
    printMatrix(M);

    // Compute RREF using singflint_rref (still uses FLINT internally, but no interpreter calls)
    matrix M_rref = singflint_rref(M, R);
    std::cout << "Computed RREF of matrix.\n";

    // Print the RREF matrix
    std::cout << "RREF matrix output:";
    printMatrix(M_rref);

    // Cleanup
    idDelete((ideal*)&M); // Free the Singular matrix (macro uses currRing)
    idDelete((ideal*)&M_rref); // Free the RREF matrix
    fmpq_mat_clear(mat);
    rDelete(R);
    return 0;
}