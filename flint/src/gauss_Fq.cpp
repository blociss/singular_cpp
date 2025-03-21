#include <flint/fmpq_mat.h>
#include <iostream>

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

// Structure to hold the result (P, U, S, rank)
typedef struct {
    fmpq_mat_t P;  // Permutation matrix
    fmpq_mat_t U;  // Upper triangular matrix
    fmpq_mat_t S;  // Row echelon form of A
    slong rank;    // Rank of the matrix
} gaussred_result_t;

// Gaussian elimination with partial pivoting over Q
gaussred_result_t gaussred_fmpq(const fmpq_mat_t A) {
    slong n = fmpq_mat_nrows(A);
    slong m = fmpq_mat_ncols(A);
    slong mr = (n < m) ? n : m;  // Minimum of rows and columns
    slong rank = 0;

    // Initialize result structure
    gaussred_result_t result;
    fmpq_mat_init(result.P, n, n);
    fmpq_mat_init(result.U, n, n);
    fmpq_mat_init(result.S, n, m);
    fmpq_mat_set(result.S, A);  // S starts as a copy of A

    // Initialize P and U as identity matrices
    fmpq_mat_one(result.P);
    fmpq_mat_one(result.U);

    fmpq_t max_coeff, pivot, c, temp;
    fmpq_init(max_coeff);
    fmpq_init(pivot);
    fmpq_init(c);
    fmpq_init(temp);

    slong col = 0;
    for (slong i = 0; i < mr && col < m; i++) {
        // Find pivot with largest absolute value in current column
        slong jp = i;
        fmpq_abs(max_coeff, fmpq_mat_entry(result.S, i, col));
        for (slong j = i + 1; j < n; j++) {
            fmpq_abs(temp, fmpq_mat_entry(result.S, j, col));
            if (fmpq_cmp(temp, max_coeff) > 0) {
                fmpq_set(max_coeff, temp);
                jp = j;
            }
        }

        // If no pivot, skip to next column
        if (fmpq_is_zero(max_coeff)) {
            col++;
            i--;
            continue;
        }

        // Swap rows if necessary
        if (jp != i) {
            fmpq_mat_swap_rows(result.S, NULL, i, jp);
            fmpq_mat_swap_rows(result.P, NULL, i, jp);
        }

        // Set pivot
        fmpq_set(pivot, fmpq_mat_entry(result.S, i, col));
     

        // Eliminate below pivot
        for (slong j = i + 1; j < n; j++) {
            if (!fmpq_is_zero(fmpq_mat_entry(result.S, j, col))) {
                fmpq_div(c, fmpq_mat_entry(result.S, j, col), pivot);
                for (slong k = col + 1; k < m; k++) {
                    fmpq_mul(temp, c, fmpq_mat_entry(result.S, i, k));
                    fmpq_sub(fmpq_mat_entry(result.S, j, k), fmpq_mat_entry(result.S, j, k), temp);
                }
                fmpq_set(fmpq_mat_entry(result.U, j, i), c);
                fmpq_set_si(fmpq_mat_entry(result.S, j, col), 0, 1);  // Set to zero correctly
            }
        }

        rank++;
        col++;
    }

    result.rank = rank;

    fmpq_clear(max_coeff);
    fmpq_clear(pivot);
    fmpq_clear(c);
    fmpq_clear(temp);

    return result;
}

int main() {
    // Initialize a 5x4 matrix over Q
    fmpq_mat_t A;
    fmpq_mat_init(A, 5, 4);

    // Fill the matrix (same values as the example)
    fmpq_set_si(fmpq_mat_entry(A, 0, 0), 1, 1);   fmpq_set_si(fmpq_mat_entry(A, 0, 1), 3, 1);   fmpq_set_si(fmpq_mat_entry(A, 0, 2), -1, 1);  fmpq_set_si(fmpq_mat_entry(A, 0, 3), 4, 1);
    fmpq_set_si(fmpq_mat_entry(A, 1, 0), 2, 1);   fmpq_set_si(fmpq_mat_entry(A, 1, 1), 5, 1);   fmpq_set_si(fmpq_mat_entry(A, 1, 2), -1, 1);  fmpq_set_si(fmpq_mat_entry(A, 1, 3), 3, 1);
    fmpq_set_si(fmpq_mat_entry(A, 2, 0), 1, 1);   fmpq_set_si(fmpq_mat_entry(A, 2, 1), 3, 1);   fmpq_set_si(fmpq_mat_entry(A, 2, 2), -1, 1);  fmpq_set_si(fmpq_mat_entry(A, 2, 3), 4, 1);
    fmpq_set_si(fmpq_mat_entry(A, 3, 0), 0, 1);   fmpq_set_si(fmpq_mat_entry(A, 3, 1), 4, 1);   fmpq_set_si(fmpq_mat_entry(A, 3, 2), -3, 1);  fmpq_set_si(fmpq_mat_entry(A, 3, 3), 1, 1);
    fmpq_set_si(fmpq_mat_entry(A, 4, 0), -3, 1);  fmpq_set_si(fmpq_mat_entry(A, 4, 1), 1, 1);   fmpq_set_si(fmpq_mat_entry(A, 4, 2), -5, 1);  fmpq_set_si(fmpq_mat_entry(A, 4, 3), -2, 1);

    print_fmpq_mat("printMatrix A:", A);

    // Perform Gaussian elimination
    gaussred_result_t result = gaussred_fmpq(A);

    // Print results
    print_fmpq_mat("P:", result.P);
    print_fmpq_mat("U:", result.U);
    print_fmpq_mat("S:", result.S);
    std::cout << "Rank: " << result.rank << "\n";

    // Clean up
    fmpq_mat_clear(A);
    fmpq_mat_clear(result.P);
    fmpq_mat_clear(result.U);
    fmpq_mat_clear(result.S);

    return 0;
}