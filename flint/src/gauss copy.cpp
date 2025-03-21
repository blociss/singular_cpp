#include <iostream>
#include <flint/fq_mat.h>
#include <flint/fmpz.h>
#include <flint/fq.h>

#include <flint/fq.h>
#include <flint/fq_mat.h>
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <flint/ulong_extras.h>
#include <iostream>

void gaussred_fq(fq_mat_t A, slong* Pivots, slong* rank, const fq_ctx_t ctx) {
    slong n = fq_mat_nrows(A, ctx);
    slong m = fq_mat_ncols(A, ctx);

    slong* L = (slong*)flint_malloc(sizeof(slong) * m);
    for (slong i = 0; i < m; i++) L[i] = m;
    for (slong i = 0; i < m; i++) Pivots[i] = -1;

    *rank = 0;
    for (slong i = 0; i < n; i++) {
        slong pivot_col = fq_mat_reduce_row(A, Pivots, L, i, ctx);
        if (pivot_col != -1) {
            (*rank)++;
        }
        std::cout << "[DEBUG] Row " << i << " reduced, pivot in column " << pivot_col << std::endl;
    }

    flint_free(L);
}

int main() {
    fmpz_t p;
    fmpz_init(p);
    fmpz_set_ui(p, 7); // Use field F_7

    fq_ctx_t ctx;
    fq_ctx_init(ctx, p, 1, "x");

    fq_mat_t A;
    fq_mat_init(A, 5, 4, ctx);

    fq_t t;
    fq_init(t, ctx);

    // Row 1
    fq_set_si(t, 1, ctx);  fq_mat_entry_set(A, 0, 0, t, ctx);
    fq_set_si(t, 3, ctx);  fq_mat_entry_set(A, 0, 1, t, ctx);
    fq_set_si(t, -1, ctx); fq_mat_entry_set(A, 0, 2, t, ctx);
    fq_set_si(t, 4, ctx);  fq_mat_entry_set(A, 0, 3, t, ctx);

    // Row 2
    fq_set_si(t, 2, ctx);  fq_mat_entry_set(A, 1, 0, t, ctx);
    fq_set_si(t, 5, ctx);  fq_mat_entry_set(A, 1, 1, t, ctx);
    fq_set_si(t, -1, ctx); fq_mat_entry_set(A, 1, 2, t, ctx);
    fq_set_si(t, 3, ctx);  fq_mat_entry_set(A, 1, 3, t, ctx);

    // Row 3
    fq_set_si(t, 1, ctx);  fq_mat_entry_set(A, 2, 0, t, ctx);
    fq_set_si(t, 3, ctx);  fq_mat_entry_set(A, 2, 1, t, ctx);
    fq_set_si(t, -1, ctx); fq_mat_entry_set(A, 2, 2, t, ctx);
    fq_set_si(t, 4, ctx);  fq_mat_entry_set(A, 2, 3, t, ctx);

    // Row 4
    fq_set_si(t, 0, ctx);  fq_mat_entry_set(A, 3, 0, t, ctx);
    fq_set_si(t, 4, ctx);  fq_mat_entry_set(A, 3, 1, t, ctx);
    fq_set_si(t, -3, ctx); fq_mat_entry_set(A, 3, 2, t, ctx);
    fq_set_si(t, 1, ctx);  fq_mat_entry_set(A, 3, 3, t, ctx);

    // Row 5
    fq_set_si(t, -3, ctx); fq_mat_entry_set(A, 4, 0, t, ctx);
    fq_set_si(t, 1, ctx);  fq_mat_entry_set(A, 4, 1, t, ctx);
    fq_set_si(t, -5, ctx); fq_mat_entry_set(A, 4, 2, t, ctx);
    fq_set_si(t, -2, ctx); fq_mat_entry_set(A, 4, 3, t, ctx);

    std::cout << "Initial Matrix A:\n";
    fq_mat_print_pretty(A, ctx);

    slong P[4]; // One for each column
    slong rank;
    gaussred_fq(A, P, &rank, ctx);

    std::cout << "\nFinal reduced matrix A:\n";
    fq_mat_print_pretty(A, ctx);
    std::cout << "Rank: " << rank << std::endl;

    fq_clear(t, ctx);
    fq_mat_clear(A, ctx);
    fq_ctx_clear(ctx);
    fmpz_clear(p);

    return 0;
}
