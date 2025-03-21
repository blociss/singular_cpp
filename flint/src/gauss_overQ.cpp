#include <flint/fq_mat.h>
#include <flint/fmpz.h>
#include <flint/fq.h>
#include <iostream>

// Print an fq_mat_t matrix for debugging
void print_fq_mat(const char* label, const fq_mat_t mat, const fq_ctx_t ctx) {
    std::cout << label << "\n-------------\n";
    for (slong i = 0; i < fq_mat_nrows(mat, ctx); i++) {
        for (slong j = 0; j < fq_mat_ncols(mat, ctx); j++) {
            fq_print_pretty(fq_mat_entry(mat, i, j), ctx);
            std::cout << "  ";
        }
        std::cout << "\n";
    }
    std::cout << "-------------\n";
}

// Manually set a matrix to the identity matrix
void fq_mat_set_one(fq_mat_t mat, const fq_ctx_t ctx) {
    fq_mat_zero(mat, ctx);
    slong n = fq_mat_nrows(mat, ctx);
    slong m = fq_mat_ncols(mat, ctx);
    slong min = (n < m) ? n : m;
    fq_t one;
    fq_init(one, ctx);
    fq_one(one, ctx);
    for (slong i = 0; i < min; i++) {
        fq_mat_entry_set(mat, i, i, one, ctx);
    }
    fq_clear(one, ctx);
}

// Structure to hold the result (P, U, S, rank)
typedef struct {
    fq_mat_t P;  // Permutation matrix
    fq_mat_t U;  // Upper triangular matrix
    fq_mat_t S;  // Row echelon form of A
    slong rank;  // Rank of the matrix
} gaussred_result_t;

// Gaussian elimination with partial pivoting over Fq
gaussred_result_t gaussred_fq(const fq_mat_t A, const fq_ctx_t ctx) {
    slong n = fq_mat_nrows(A, ctx);
    slong m = fq_mat_ncols(A, ctx);
    slong mr = (n < m) ? n : m;  // Minimum of rows and columns
    slong rank = 0;

    // Initialize result structure
    gaussred_result_t result;
    fq_mat_init(result.P, n, n, ctx);
    fq_mat_init(result.U, n, n, ctx);
    fq_mat_init(result.S, n, m, ctx);
    fq_mat_set(result.S, A, ctx);  // S starts as a copy of A

    // Initialize P and U as identity matrices
    fq_mat_set_one(result.P, ctx);  // Use custom function
    fq_mat_set_one(result.U, ctx);  // Use custom function

    fq_t max_coeff, pivot, c, temp, zero;
    fq_init(max_coeff, ctx);
    fq_init(pivot, ctx);
    fq_init(c, ctx);
    fq_init(temp, ctx);
    fq_init(zero, ctx);
    fq_zero(zero, ctx);  // Initialize zero value

    slong col = 0;
    for (slong i = 0; i < mr && col < m; i++) {
        // Find pivot (first non-zero entry in current column)
        slong jp = i;
        fq_set(max_coeff, fq_mat_entry(result.S, i, col), ctx);
        for (slong j = i + 1; j < n; j++) {
            if (!fq_is_zero(fq_mat_entry(result.S, j, col), ctx)) {
                jp = j;
                fq_set(max_coeff, fq_mat_entry(result.S, j, col), ctx);
                break;  // Take the first non-zero entry as pivot
            }
        }

        // If no pivot, skip to next column
        if (fq_is_zero(max_coeff, ctx)) {
            col++;
            i--;
            continue;
        }

        // Swap rows if necessary
        if (jp != i) {
            fq_mat_swap_rows(result.S, NULL, i, jp, ctx);
            fq_mat_swap_rows(result.P, NULL, i, jp, ctx);
            std::cout << "[DEBUG] Swapped row " << i << " with row " << jp << std::endl;
        }

        // Set pivot
        fq_set(pivot, fq_mat_entry(result.S, i, col), ctx);
        std::cout << "[DEBUG] Row " << i << " reduced, pivot in column " << col << ": ";
        fq_print_pretty(pivot, ctx);
        std::cout << std::endl;

        // Eliminate below pivot
        for (slong j = i + 1; j < n; j++) {
            if (!fq_is_zero(fq_mat_entry(result.S, j, col), ctx)) {
                fq_div(c, fq_mat_entry(result.S, j, col), pivot, ctx);
                for (slong k = col + 1; k < m; k++) {
                    fq_mul(temp, c, fq_mat_entry(result.S, i, k), ctx);
                    fq_sub(fq_mat_entry(result.S, j, k), fq_mat_entry(result.S, j, k), temp, ctx);
                }
                fq_set(fq_mat_entry(result.U, j, i), c, ctx);
                fq_set(fq_mat_entry(result.S, j, col), zero, ctx);  // Set to zero
            }
        }

        rank++;
        col++;
    }

    result.rank = rank;

    fq_clear(max_coeff, ctx);
    fq_clear(pivot, ctx);
    fq_clear(c, ctx);
    fq_clear(temp, ctx);
    fq_clear(zero, ctx);

    return result;
}

int main() {
    // Initialize finite field F_7
    fmpz_t p;
    fmpz_init(p);
    fmpz_set_ui(p, 7);  // Field F_7

    fq_ctx_t ctx;
    fq_ctx_init(ctx, p, 1, "x");  // F_7, degree 1 extension

    // Initialize a 5x4 matrix over F_7
    fq_mat_t A;
    fq_mat_init(A, 5, 4, ctx);

    // Temporary variable for setting entries
    fq_t t;
    fq_init(t, ctx);

    // Fill the matrix (same values as the example, reduced mod 7)
    fq_set_si(t, 1, ctx);   fq_mat_entry_set(A, 0, 0, t, ctx);  fq_set_si(t, 3, ctx);   fq_mat_entry_set(A, 0, 1, t, ctx);  fq_set_si(t, -1, ctx);  fq_mat_entry_set(A, 0, 2, t, ctx);  fq_set_si(t, 4, ctx);   fq_mat_entry_set(A, 0, 3, t, ctx);
    fq_set_si(t, 2, ctx);   fq_mat_entry_set(A, 1, 0, t, ctx);  fq_set_si(t, 5, ctx);   fq_mat_entry_set(A, 1, 1, t, ctx);  fq_set_si(t, -1, ctx);  fq_mat_entry_set(A, 1, 2, t, ctx);  fq_set_si(t, 3, ctx);   fq_mat_entry_set(A, 1, 3, t, ctx);
    fq_set_si(t, 1, ctx);   fq_mat_entry_set(A, 2, 0, t, ctx);  fq_set_si(t, 3, ctx);   fq_mat_entry_set(A, 2, 1, t, ctx);  fq_set_si(t, -1, ctx);  fq_mat_entry_set(A, 2, 2, t, ctx);  fq_set_si(t, 4, ctx);   fq_mat_entry_set(A, 2, 3, t, ctx);
    fq_set_si(t, 0, ctx);   fq_mat_entry_set(A, 3, 0, t, ctx);  fq_set_si(t, 4, ctx);   fq_mat_entry_set(A, 3, 1, t, ctx);  fq_set_si(t, -3, ctx);  fq_mat_entry_set(A, 3, 2, t, ctx);  fq_set_si(t, 1, ctx);   fq_mat_entry_set(A, 3, 3, t, ctx);
    fq_set_si(t, -3, ctx);  fq_mat_entry_set(A, 4, 0, t, ctx);  fq_set_si(t, 1, ctx);   fq_mat_entry_set(A, 4, 1, t, ctx);  fq_set_si(t, -5, ctx);  fq_mat_entry_set(A, 4, 2, t, ctx);  fq_set_si(t, -2, ctx);  fq_mat_entry_set(A, 4, 3, t, ctx);

    print_fq_mat("printMatrix A:", A, ctx);

    // Perform Gaussian elimination
    gaussred_result_t result = gaussred_fq(A, ctx);

    // Print results
    print_fq_mat("P:", result.P, ctx);
    print_fq_mat("U:", result.U, ctx);
    print_fq_mat("S:", result.S, ctx);
    std::cout << "Rank: " << result.rank << "\n";

    // Clean up
    fq_mat_clear(A, ctx);
    fq_mat_clear(result.P, ctx);
    fq_mat_clear(result.U, ctx);
    fq_mat_clear(result.S, ctx);
    fq_clear(t, ctx);
    fq_ctx_clear(ctx);
    fmpz_clear(p);

    return 0;
}