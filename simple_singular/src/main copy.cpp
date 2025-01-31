#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>  // For strdup

int main()
{
siInit((char *)"/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-snapshot_22_03-5jvwtprazqirywu2triw6rprjazzi3so/lib/libSingular.so");

    // Define variable names
    char **n = (char**)omalloc(3 * sizeof(char*));
    n[0] = omStrDup("x");
    n[1] = omStrDup("y");
    n[2] = omStrDup("z");

    // Create ring Z/32003[x,y,z]
    ring R = rDefault(32003, 3, n);
    rChangeCurrRing(R);  // Set R as the current ring

    // Create polynomials
    poly p1 = p_ISet(1, R);
    poly p2 = p_ISet(2, R);
    pWrite(p2);
    pSetExp(p2, 1, 3);  // Set exponent of x to 3
    pSetExp(p2, 2, 4);  // Set exponent of y to 4
    pSetExp(p2, 3, 2);  // Set exponent of z to 2
    pSetm(p2);          // Normalize the monomial

    // Print p1 + p2
    pWrite(p1); printf(" + "); pWrite(p2); printf("\n");

    // Compute p1 + p2 safely
    poly p1_copy = p_Copy(p1, R);
    p1 = p_Add_q(p1, p2, R);
    p_Delete(&p1_copy, R);
    p2 = NULL;

    pWrite(p1);
    
    // Clean up
    p_Delete(&p1, R);
    rKill(R);

    omFree(n[0]); omFree(n[1]); omFree(n[2]); omFree(n);

    return 0;
}
