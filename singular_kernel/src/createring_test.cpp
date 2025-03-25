#include <iostream>
#include <vector>
#include <string>
#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/simpleideals.h>


int main()
{
    // 1) Initialize the Singular library:
    //    (Omit mp_set_memory_functions if your version doesn't export those symbols)
    siInit((char*)"");

    // 2) Create the base coefficient field Q:
    coeffs baseCoeff = nInitChar(n_Q, nullptr);
    if (!baseCoeff)
    {
        std::cerr << "Failed to initialize base coefficient field (QQ).\n";
        return 1;
    }

    // 3) List of 11 variables: p(1),p(2),p(3),p(4),q(1),...,q(7)
    std::vector<std::string> varNames;
    for (int i = 1; i <= 4; i++)
        varNames.push_back("p(" + std::to_string(i) + ")");
    for (int i = 1; i <= 7; i++)
        varNames.push_back("q(" + std::to_string(i) + ")");

    // Convert to char* array for rDefault:
    std::vector<char*> cVarNames;
    cVarNames.reserve(varNames.size());
    for (auto &s : varNames)
        cVarNames.push_back(const_cast<char*>(s.c_str()));

    // 4) Create the ring: QQ[x1..x11] with dp order
    //    Here x1..x4 = p(1)..p(4), x5..x11 = q(1)..q(7)
    ring R = rDefault(baseCoeff, (int)varNames.size(), cVarNames.data(), ringorder_dp);
    rComplete(R);
    rChangeCurrRing(R);

    // Print ring info:
    std::cout << "Ring R created: " << rString(R) << std::endl;
    rWrite(R); // more detailed ring info

    // 5) Build an ideal I of 7 generators:
    ideal I = idInit(7, 1); // 7 polynomials, 1 module component (typical)

    // Helper: for quick "constant" polynomials:
    auto polyConst = [&](long c) {
        return p_ISet(c, R); // e.g. p_ISet(1,R) yields the polynomial "1"
    };

    //---  Generator I[0] = 2*p(1) + p(4)
    I->m[0] = polyConst(2);             // 2
    p_SetExp(I->m[0], 1, 1, R);         // sets exponent of var1 => p(1)
    p_Setm(I->m[0], R);                 // finalize monomial
    poly temp = polyConst(1);           // 1
    p_SetExp(temp, 4, 1, R);            // +p(4)
    p_Setm(temp, R);
    I->m[0] = p_Add_q(I->m[0], temp, R); // q(4)+q(1)
    // Now add p(1):
    temp = p_ISet(0, R); // we will set the coefficient to p(1)
    // A simpler approach: just do p_SetExp(temp,0,0,R) => 1, then multiply by p(1),
    // but let's demonstrate n_Param if we want the 1st "p" variable as a parameter:
    // Instead, for a purely polynomial ring, we can do: p_SetExp(temp, 1, 1, R) if p(1) is var1. 
    // But let's do a direct approach:
    // Because we used 4 p(...) first, they occupy var indices 1..4, so p(1) is variable #1 in the ring:
    temp = polyConst(1);
    p_SetExp(temp, 1, 1, R);  // p(1) is var #1
    p_Setm(temp, R);
    I->m[0] = p_Add_q(I->m[0], temp, R); // q(4) + q(1) + p(1)

    //---  Generator I[1] = -p(1) + p(2) + q(1)
    I->m[1] = polyConst(-1);
    p_SetExp(I->m[1], 1, 1, R);  // -p(1)
    p_Setm(I->m[1], R);
    temp = polyConst(1);       // 1
    p_SetExp(temp, 2, 1, R);    // +p(2)
    p_Setm(temp, R);
    I->m[1] = p_Add_q(I->m[1], temp, R);
    // + q(1):
    temp = polyConst(1);
    p_SetExp(temp, 5, 1, R); // q(1) is var #5
    p_Setm(temp, R);
    I->m[1] = p_Add_q(I->m[1], temp, R);

    //---  Generator I[2] = p(2)*q(2) - q(1) + q(3)
    I->m[2] = polyConst(1);
    p_SetExp(I->m[2], 2, 1, R); // p(2)
    p_SetExp(I->m[2], 6, 1, R); // *q(2)
    p_Setm(I->m[2], R);
    temp = polyConst(-1);
    p_SetExp(temp, 5, 1, R); // -q(1)
    p_Setm(temp, R);
    I->m[2] = p_Add_q(I->m[2], temp, R);
    temp = polyConst(1);
    p_SetExp(temp, 7, 1, R); // +q(3)
    p_Setm(temp, R);
    I->m[2] = p_Add_q(I->m[2], temp, R);

    //---  Generator I[3] = p(4) + q(3) - q(7)
    I->m[3] = polyConst(1);
    p_SetExp(I->m[3], 4, 1, R); // p(4)
    p_Setm(I->m[3], R);
    temp = polyConst(1);
    p_SetExp(temp, 7, 1, R); // +q(3)
    p_Setm(temp, R);
    I->m[3] = p_Add_q(I->m[3], temp, R);
    temp = polyConst(-1);
    p_SetExp(temp, 11, 1, R); // -q(7)
    p_Setm(temp, R);
    I->m[3] = p_Add_q(I->m[3], temp, R);

    //---  Generator I[4] = p(3) - q(3) + q(6)
    I->m[4] = polyConst(1);
    p_SetExp(I->m[4], 3, 1, R); // p(3)
    p_Setm(I->m[4], R);
    temp = polyConst(-1);
    p_SetExp(temp, 7, 1, R); // -q(3)
    p_Setm(temp, R);
    I->m[4] = p_Add_q(I->m[4], temp, R);
    temp = polyConst(1);
    p_SetExp(temp, 10, 1, R); // +q(6)
    p_Setm(temp, R);
    I->m[4] = p_Add_q(I->m[4], temp, R);

    //---  Generator I[5] = -q(2) - q(4) - q(6)
    I->m[5] = polyConst(-1);
    p_SetExp(I->m[5], 6, 1, R); // -q(2)
    p_Setm(I->m[5], R);
    temp = polyConst(-1);
    p_SetExp(temp, 8, 1, R); // -q(4)
    p_Setm(temp, R);
    I->m[5] = p_Add_q(I->m[5], temp, R);
    temp = polyConst(-1);
    p_SetExp(temp, 10, 1, R); // -q(6)
    p_Setm(temp, R);
    I->m[5] = p_Add_q(I->m[5], temp, R);

    //---  Generator I[6] = p(1)+p(2)+p(3)+p(4)
    // p(1)->var1, p(2)->var2, p(3)->var3, p(4)->var4
    I->m[6] = polyConst(0);
    for(int v = 1; v <= 4; v++) {
      temp = polyConst(1);
      p_SetExp(temp, v, 1, R);
      p_Setm(temp, R);
      I->m[6] = p_Add_q(I->m[6], temp, R);
    }

    // 6) Print the balancing ideal:
    std::cout << "\nBalancing ideal I:\n";
    for(int i = 0; i < IDELEMS(I); i++)
    {
        char* s = p_String(I->m[i], R);
        std::cout << "I[" << i << "] = " << s << std::endl;
        omFree(s);
    }

    // 7) Compute standard basis:
    ideal i_std = kStd(I, nullptr, testHomog, nullptr);
    if (i_std)
    {
        std::cout << "\nStandard basis of I:\n";
        for(int i = 0; i < IDELEMS(i_std); i++)
        {
            if (i_std->m[i] != NULL) {
                char* s = p_String(i_std->m[i], R);
                if (strcmp(s, "0") != 0) {
                    std::cout << "i_std[" << i << "] = " << s << std::endl;
                }
                omFree(s);
            }
        }
        id_Delete(&i_std, R);
    }

    // 8) Cleanup
    id_Delete(&I, R);
    rKill(R);
    

    return 0;
}
