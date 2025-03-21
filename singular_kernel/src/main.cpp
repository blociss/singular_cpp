#include <singular/Singular/libsingular.h>
#include <singular/kernel/polys.h>
#include <singular/kernel/ideals.h>
#include <iostream>

// Convert a number to a string for debugging
std::string numberToString(number c, ring R) {
    StringSetS("");              // Initialize string buffer
    n_Write(c, R->cf);           // Write number into buffer
    char* singularStr = StringEndS(); // Retrieve buffer content
    std::string result(singularStr);  // Convert to std::string
    omFree(singularStr);              // Free Singular-allocated memory
    return result;
}

// Print a number as a polynomial for debugging
void printNumber(const number z, ring R) {
    if (n_IsZero(z, R->cf)) {
        std::cout << "number = 0\n";
    } else {
        poly p = p_One(R);
        p_SetCoeff(p, n_Copy(z, R->cf), R);
        p_Setm(p, R);
        std::cout << "number = " << p_String(p, R) << "\n";
        p_Delete(&p, R);
    }
}

// Print a matrix for debugging
void printMatrix(const matrix m, ring R) {
    int rr = MATROWS(m);
    int cc = MATCOLS(m);
    std::cout << "\n-------------\n";
    for (int r = 1; r <= rr; r++) {
        for (int c = 1; c <= cc; c++) {
            char* str = p_String(MATELEM(m, r, c), R);
            std::cout << str << "  ";
            omFree(str);
        }
        std::cout << "\n";
    }
    std::cout << "-------------\n";
}

// Compute the absolute value of a polynomial's coefficient
number absValue(poly p, ring R) {
    if (p == NULL) return n_Init(0, R->cf);
    number result = n_Copy(p_GetCoeff(p, R), R->cf);
    if (!n_GreaterZero(result, R->cf)) {
        number neg = n_InpNeg(result, R->cf);
        n_Delete(&result, R->cf);
        result = neg;
    }
    return result;
}



lists gaussred_pivot(matrix A) {
    ring r = currRing;
    if (!r) {
        std::cerr << "[ERROR] No current ring defined.\n";
        return NULL;
    }

    int n = MATROWS(A), m = MATCOLS(A);
    int mr = (n < m) ? n : m;
    int k = 0, rank = 0;

    matrix P = mpNew(n, n);
    matrix U = mpNew(n, n);
    matrix S = mp_Copy(A, r);

    for (int i = 1; i <= n; i++) {
        MATELEM(P, i, i) = pISet(1);
        MATELEM(U, i, i) = pISet(1);
    }

    for (int i = 1; i <= mr; i++) {
        int col = i + k;
        if (col > m) break;

        int jp = i;
        number max_coeff = n_Copy(absValue(MATELEM(S, i, col), r), r->cf);

        for (int j = i + 1; j <= n; j++) {
            number c = n_Copy(absValue(MATELEM(S, j, col), r), r->cf);
            if (n_Greater(c, max_coeff, r->cf)) {
                n_Delete(&max_coeff, r->cf);
                max_coeff = c;
                jp = j;
            } else {
                n_Delete(&c, r->cf);
            }
        }
        n_Delete(&max_coeff, r->cf);


        if (jp != i) {
            for (int j = 1; j <= m; j++)
                std::swap(MATELEM(S, i, j), MATELEM(S, jp, j));
            for (int j = 1; j <= n; j++)
                std::swap(MATELEM(P, i, j), MATELEM(P, jp, j));
        }

        poly pivot = MATELEM(S, i, col);
        if (!pivot) {
            k++;
            i--;
            continue;
        }

        number pivot_coeff = p_GetCoeff(pivot, r);

        for (int j = i + 1; j <= n; j++) {
            poly num = MATELEM(S, j, col);
            if (!num) {
                continue;
            }

            number num_coeff = p_GetCoeff(num, r);
            number c = n_Div(num_coeff, pivot_coeff, r->cf);
           

            for (int l = col; l <= m; l++) {
                poly s_il = pCopy(MATELEM(S, i, l));
                poly s_jl = pCopy(MATELEM(S, j, l));
             

                poly prod = s_il ? p_Mult_nn(s_il, c, r) : NULL;


                poly sub_result = NULL;

                if (s_jl && prod) {
                    sub_result = p_Sub(p_Copy(s_jl, r), p_Copy(prod, r), r);
                } else if (s_jl && !prod) {
                    sub_result = p_Copy(s_jl, r);
                } else if (!s_jl && prod) {
                    sub_result = p_Neg(p_Copy(prod, r), r);
                }


                if (MATELEM(S, j, l)) p_Delete(&MATELEM(S, j, l), r);
                MATELEM(S, j, l) = sub_result;

                if (prod) p_Delete(&prod, r);
            }

            if (MATELEM(S, j, col)) {
                p_Delete(&MATELEM(S, j, col), r);
                MATELEM(S, j, col) = NULL;
            }

            if (MATELEM(U, j, i)) {
                p_Delete(&MATELEM(U, j, i), r);
                MATELEM(U, j, i) = NULL;
            }
            MATELEM(U, j, i) = p_NSet(c, r);
        }

        rank = i;
    }

    lists Z = (lists)omAlloc0Bin(slists_bin);
    Z->Init(4);
    Z->m[0].rtyp = MATRIX_CMD; Z->m[0].data = P;
    Z->m[1].rtyp = MATRIX_CMD; Z->m[1].data = U;
    Z->m[2].rtyp = MATRIX_CMD; Z->m[2].data = S;
    Z->m[3].rtyp = INT_CMD;    Z->m[3].data = (void*)(long)rank;

    return Z;
}


int main() {
    // Initialize Singular library (adjust path as needed)
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");

    // Define a ring with one variable x over Q
    char var_x[] = "x";
    char* vars[] = {var_x};
    ring r = rDefault(0, 1, vars);
    rComplete(r);
    rChangeCurrRing(r);

    // Create a 5x4 matrix A
    matrix A = mpNew(5, 4);
    MATELEM(A, 1, 1) = pISet(1);  MATELEM(A, 1, 2) = pISet(3);  MATELEM(A, 1, 3) = pISet(-1); MATELEM(A, 1, 4) = pISet(4);
    MATELEM(A, 2, 1) = pISet(2);  MATELEM(A, 2, 2) = pISet(5);  MATELEM(A, 2, 3) = pISet(-1); MATELEM(A, 2, 4) = pISet(3);
    MATELEM(A, 3, 1) = pISet(1);  MATELEM(A, 3, 2) = pISet(3);  MATELEM(A, 3, 3) = pISet(-1); MATELEM(A, 3, 4) = pISet(4);
    MATELEM(A, 4, 1) = pISet(0);  MATELEM(A, 4, 2) = pISet(4);  MATELEM(A, 4, 3) = pISet(-3); MATELEM(A, 4, 4) = pISet(1);
    MATELEM(A, 5, 1) = pISet(-3); MATELEM(A, 5, 2) = pISet(1);  MATELEM(A, 5, 3) = pISet(-5); MATELEM(A, 5, 4) = pISet(-2);
std::cout<<" printMatrix A= ";
printMatrix(A,r);
    // Perform Gaussian elimination
    lists Z = gaussred_pivot(A);
    if (Z) {
        matrix P = (matrix)Z->m[0].Data();
        matrix U = (matrix)Z->m[1].Data();
        matrix S = (matrix)Z->m[2].Data();
        long rank = (long)Z->m[3].Data();

        std::cout << "P:\n"; printMatrix(P, r);
        std::cout << "U:\n"; printMatrix(U, r);
        std::cout << "S:\n"; printMatrix(S, r);
        std::cout << "Rank: " << rank << "\n";

        // Clean up the list
        leftv wrapper = (leftv)omAlloc0Bin(sleftv_bin);
        wrapper->rtyp = LIST_CMD;
        wrapper->data = Z;
        wrapper->CleanUp();
        omFreeBin(wrapper, sleftv_bin);
    }

    // Clean up
    mp_Delete(&A, r);
    rKill(r);
    return 0;
}