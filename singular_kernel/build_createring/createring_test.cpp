#include <Singular/libsingular.h>
#include <iostream>

int main() {
    // Initialize Singular
    siInit((char *)"createring_test");

    // Create first ring
    ring P = rDefault(0, 3, "x,y,z");
    rComplete(P);
    rChangeCurrRing(P);
    
    std::cout << "Created ring P: " << rString(P) << std::endl;

    // Create test polynomial in P
    poly p = p_ISet(1, P);
    p_SetExp(p, 1, 1, P); // Set x^1
    p_Setm(p, P);
    std::cout << "Test polynomial in P: " << p_String(p, P) << std::endl;

    // Create second ring
    ring R = rCopy0(P);
    rComplete(R);

    std::cout << "Created ring R: " << rString(R) << std::endl;

    // Try different mapping approaches
    std::cout << "\nTesting mapping approaches:\n";

    // 1. Using prCopyR
    poly p1 = prCopyR(p, P, R);
    std::cout << "1. prCopyR result: " << (p1 ? p_String(p1, R) : "NULL") << std::endl;

    // 2. Using p_PermPoly
    int *perm = (int*)omAlloc0((rVar(P) + 1) * sizeof(int));
    for (int i = 0; i <= rVar(P); i++) perm[i] = i;
    poly p2 = p_PermPoly(p, perm, P, R, NULL, NULL, 0, FALSE);
    std::cout << "2. p_PermPoly result: " << (p2 ? p_String(p2, R) : "NULL") << std::endl;

    // 3. Using nSetMap and manual copying
    nMapFunc nMap = n_SetMap(P->cf, R->cf);
    if (nMap) {
        poly p3 = p_Init(R);
        number n = nMap(pGetCoeff(p), R->cf);
        p_SetCoeff0(p3, n, R);
        for (int i = 1; i <= rVar(P); i++) {
            p_SetExp(p3, i, p_GetExp(p, i, P), R);
        }
        p_Setm(p3, R);
        std::cout << "3. Manual mapping result: " << p_String(p3, R) << std::endl;
        p_Delete(&p3, R);
    } else {
        std::cout << "3. Manual mapping failed: nMap is NULL" << std::endl;
    }

    // Clean up
    omFreeSize((ADDRESS)perm, (rVar(P) + 1) * sizeof(int));
    p_Delete(&p, P);
    p_Delete(&p1, R);
    p_Delete(&p2, R);
    rDelete(R);
    rDelete(P);

    return 0;
}
