#include <singular/Singular/libsingular.h>
#include <iostream>

int main() {
    // Initialize Singular
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Construct the ring Z/32003[x1, x2, ..., x52]
    char **n = (char**)omalloc(52 * sizeof(char*));  // 52 variables
    for (int i = 0; i < 52; i++) {
        n[i] = omStrDup(("x" + std::to_string(i+1)).c_str());  // Generate variable names x1, x2, ..., x52
    }

    ring R = rDefault(32003, 52, n);
    rChangeCurrRing(R);  // Set the current ring to R

    // Create and initialize polynomials p1, p2, ..., p52 dynamically
    ideal L = idInit(52, 1);  // 52 generators for the ideal
    for (int i = 0; i < 52; i++) {
        poly p = p_ISet( 1, R);  // Set polynomial with degree (1)
      
        pSetExp(p, i + 1,  1);  // Set exponent for the i+1-th variable
        
        L->m[i] = pCopy(p);  // Copy the polynomial into the ideal
    }

    // Call a Singular procedure (for example, computing the Grobner basis)
    std::cout << "Calling Singular procedure to compute Grobner basis..." << std::endl;
    ideal G = groebner(L);  // Compute the Grobner basis of the ideal L
    
    // Print the Grobner basis
    std::cout << "Grobner basis of ideal L: \n";
    for (int i = 0; i < IDELEMS(G); i++) {
        pWrite(G->m[i]);
        std::cout << std::endl;
    }

    
    ideal variety_ideal_std = kStd(L, currRing->qideal, testHomog, NULL);
    /* std::cout << "Variety of the ideal: ";
    pWrite(variety_ideal_std->m[1]);  // Print first generator of the variety ideal
    std::cout << std::endl;
 */
    // Print the ideal's generators of the variety
    std::cout << "Generators of ideal variety_ideal_std: \n";
    int tt = IDELEMS(variety_ideal_std);
    for (int i = 0; i < tt; i++) {
        pWrite(variety_ideal_std->m[i]);
        std::cout << " , ";
    }

    // Clean up
    for (int i = 0; i < 52; i++) {
        pDelete(&L->m[i]);  // Clean up the polynomials in the ideal
    }
    idDelete(&L);  // Delete the ideal
    idDelete(&G);  // Delete the Grobner basis
    rKill(R);  // Clean up the ring

    return 0;
}
