#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/simpleideals.h>
#include "singular_functions.hpp"

std::pair<int, lists> std_gpi(leftv arg1) {
    // Validate input argument
    if (!arg1 || !arg1->Data()) {
        std::cerr << "Error: Invalid input argument!" << std::endl;
        return {0, nullptr};
    }

    // Extract input list
    lists input = (lists)(arg1->Data());
    if (!input || input->nr < 3) {
        std::cerr << "Error: Input list is invalid or too small!" << std::endl;
        return {0, nullptr};
    }

    // Extract tmp list
    lists tmp = (lists)(input->m[3].Data());
    if (!tmp || tmp->nr < 0) {
        std::cerr << "Error: Invalid tmp list!" << std::endl;
        return {0, nullptr};
    }

    // Extract ideal M
    ideal M = (ideal)tmp->m[0].Data();
    if (!M) {
        std::cerr << "Error: Ideal M is null!" << std::endl;
        return {0, nullptr};
    }

    // Check size of M
    int p = IDELEMS(M);
    if (p <= 0) {
        std::cerr << "Error: Ideal M has no elements!" << std::endl;
        return {0, nullptr};
    }

    // Set pr dynamically based on M
    int pr = 16;; // Use the size of M to determine pr
    if (pr > IDELEMS(M)) {
        std::cerr << "Error: pr exceeds the size of M!" << std::endl;
        return {0, nullptr};
    }

    // Initialize ideal L
    ideal L = idInit(pr, 1);
    if (!L) {
        std::cerr << "Error: Failed to initialize ideal L!" << std::endl;
        return {0, nullptr};
    }

    // Copy polynomials from M to L
    for (int i = 0; i < pr; i++) {
        if (!M->m[i] || !p_Test((poly)M->m[i], currRing)) {
            std::cerr << "Error: M->m[" << i << "] is invalid!" << std::endl;
            return {0, nullptr};
        }
        L->m[i] = pCopy(M->m[i]);
       // std::cout << "L->m[" << i << "]: " << pString((poly)L->m[i]) << std::endl;
    }


   //ideal MM=kStd(L,currRing->qideal, testHomog, NULL);
        omUpdateInfo();
        std::cout << "used mem: " << om_Info.UsedBytes << std::endl;
    ideal Li = kStd(L, currRing->qideal,  testHomog,  NULL);
    if (!Li) {
        std::cerr << "Error: kStd returned null!" << std::endl;
        return {0, nullptr};
    }
 omUpdateInfo();
        std::cout << "used mem: " << om_Info.UsedBytes << std::endl;
    // Check size of Li
    int tt = IDELEMS(Li);
    std::cout << "Li_size: " << tt << std::endl;
  

  // Debugging output for Li
    for (int i = 0; i < tt; i++) {
        std::cout << "Li->m[" << i << "]: " << pString((poly)Li->m[i]) << std::endl;
    } 
       // std::cout << "Li->m[" << 38 << "]: " << pString((poly)Li->m[37]) << std::endl;

    // Prepare the output token
    lists output = (lists)omAlloc0Bin(slists_bin);
    output->Init(4);

    lists t = (lists)omAlloc0Bin(slists_bin);
    t->Init(2);
    t->m[0].rtyp = STRING_CMD; t->m[0].data = strdup("generators");
    t->m[1].rtyp = STRING_CMD; t->m[1].data = strdup("module_std");
    output->m[1].rtyp = LIST_CMD; output->m[1].data = t;
    output->m[0].rtyp = RING_CMD; output->m[0].data = currRing;
    output->m[2].rtyp = RING_CMD; output->m[2].data = currRing;

    t = (lists)omAlloc0Bin(slists_bin);
    t->Init(tt); // Use the size of Li to initialize t
    for (int i = 0; i < tt; i++) {
        t->m[i].rtyp = POLY_CMD; t->m[i].data = pCopy((poly)Li->m[i]);
    }
    output->m[3].rtyp = LIST_CMD; output->m[3].data = t;

    // Cleanup

    return {p, output};
}

int main() {
    
        // Initialize Singular with the appropriate library.
        siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

        // Specify the path to your SSI file.
        std::string filePath = "/home/atraore/gpi/try_gpi/singular_cpp/src/waas_54549_1738847576961148731";

        // Obtain an identifier string (e.g., hostname and pid) using worker()
        std::string ids = worker();

        std::pair<int, lists> input = deserialize(filePath, ids);

       

     std::string out_filename;
    std::string out_filename1;
     ScopedLeftv args(input.first, lCopy(input.second));
    //std::string function_name2 = "stdBasis";

    lists Token = (lists)(args.leftV()->data);

    int L_size = lSize(Token) + 1;
    for (int i = 0; i < L_size; i++) {
        sleftv& listElement = Token->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
           // std::cout << "Input: NULL" << std::endl;
        }
        else if (i == 3) {
            out_filename1 = listElement.String();
             //std::cout << "out_filename1= " << out_filename1 << std::endl;
        }
    }
        std::pair<int, lists> out;

  out = std_gpi(args.leftV());

    return 0;
}
