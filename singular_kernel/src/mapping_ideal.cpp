
LabeledGraph eliminateVariables(LabeledGraph G) {
    std::cout << "[DEBUG] Entering eliminateVariables" << std::endl;
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
  ring savedRing = currRing;

// Compute balancing ideal in G.over
std::cout << "[DEBUG] Switching to ring: " << rString(G.over) << std::endl;
rChangeCurrRing(G.over);
ideal I = balancingIdeal(G);
std::cout << "[DEBUG] Ideal computed" << std::endl;
for (int i = 0; i < IDELEMS(I); i++) {
    if (!I->m[i]) continue;
    char* s = p_String(I->m[i], G.over);
    std::cout << "[DEBUG] Ideal element I[" << i << "] = " << s << std::endl;
    omFree(s);
}

// Map ideal to G.overpoly
std::cout << "[DEBUG] Mapping ideal to G.overpoly..." << std::endl;

// Create identity permutation for variables
int nvars = rVar(G.over);
int *perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
for (int i = 0; i <= nvars; i++) {
    perm[i] = i;
}
std::cout << "[DEBUG] nvars = " << nvars << std::endl;
// Create mapped ideal
ideal I_mapped = idInit(IDELEMS(I), 1);
std::cout << "[DEBUG] I_mapped initialized" << std::endl;
// Debug permutation array
std::cout << "[DEBUG] Permutation array: ";
for (int i = 0; i < nvars; i++) {
    std::cout << perm[i] << " ";
}
std::cout << std::endl;

// Verify rings
std::cout << "[DEBUG] Source ring (G.over): " << rString(G.over) << std::endl;
std::cout << "[DEBUG] Target ring (G.overpoly): " << rString(G.overpoly) << std::endl;

// Map each polynomial using p_PermPoly with safety checks
for (int i = 0; i < IDELEMS(I); i++) {
    std::cout << "[DEBUG] Processing ideal element " << i << std::endl;
    
    if (!I || !I->m) {
        std::cout << "[ERROR] Invalid ideal or ideal elements array" << std::endl;
        continue;
    }

    if (!I->m[i]) {
        std::cout << "[DEBUG] Skipping null element at position " << i << std::endl;
        continue;
    }

    std::cout << "[DEBUG] Mapping ideal element I[" << i << "] = " << p_String(I->m[i], G.over) << std::endl;
    
    // Verify source polynomial
    if (!p_Test(I->m[i], G.over)) {
        std::cout << "[ERROR] Invalid source polynomial at position " << i << std::endl;
        continue;
    }

    // Create a new polynomial in G.overpoly
    poly mapped = p_Init(G.overpoly);
    p_SetCoeff(mapped, n_Init(1, G.overpoly->cf), G.overpoly);
    
    // Copy each term
    for (poly p = I->m[i]; p != NULL; p = p->next) {
        poly term = p_Init(G.overpoly);
        
        // Copy exponents using permutation
        for (int j = 1; j <= rVar(G.over); j++) {
            int exp = p_GetExp(p, j, G.over);
            if (exp > 0) {
                p_SetExp(term, perm[j-1] + 1, exp, G.overpoly);
            }
        }
        p_Setm(term, G.overpoly);
        
        // Set coefficient to 1 for now (we'll handle parameters separately)
        p_SetCoeff(term, n_Init(1, G.overpoly->cf), G.overpoly);
        
        // Add to result
        mapped = p_Add_q(mapped, term, G.overpoly);
    }
    
    if (!mapped) {
        std::cout << "[ERROR] Failed to create mapped polynomial at position " << i << std::endl;
        continue;
    }

    // Verify mapped polynomial
    if (!p_Test(mapped, G.overpoly)) {
        std::cout << "[ERROR] Invalid mapped polynomial at position " << i << std::endl;
        p_Delete(&mapped, G.overpoly);
        continue;
    }

    I_mapped->m[i] = mapped;
    char* s = p_String(mapped, G.overpoly);
    std::cout << "[DEBUG] Successfully mapped to: " << s << std::endl;
    omFree(s);
}

// Clean up
omFreeSize((ADDRESS)perm, (nvars + 1) * sizeof(int));

std::cout << "[DEBUG] I_mapped computed" << std::endl;
for (int i = 0; i < IDELEMS(I_mapped); i++) {
    if (!I_mapped->m[i]) continue;
    char* s = p_String(I_mapped->m[i], G.overpoly);
    std::cout << "[DEBUG] I_mapped[" << i << "] = " << s << std::endl;
    omFree(s);
}

// Compute reduced standard basis
std::cout << "[DEBUG] Computing standard basis in G.overpoly..." << std::endl;

// Verify the ideal before computing standard basis
if (!I_mapped || !I_mapped->m) {
    std::cout << "[ERROR] Invalid ideal before computing standard basis" << std::endl;
    G.over = NULL;
    G.overpoly = NULL;
    return G;
}

// Set computation parameters
BOOLEAN redSB = TRUE; // Mimic option(redSB)
BOOLEAN testHomog = FALSE; // Don't test for homogeneity

// Create a copy of the ideal for safety
ideal I_copy = idCopy(I_mapped);

// Compute standard basis with safety checks
ideal I_std = NULL;
try {
    std::cout << "[DEBUG] Starting standard basis computation..." << std::endl;
    std::cout << "[DEBUG] I_copy->rank = " << I_copy->rank << std::endl;
    std::cout << "[DEBUG] I_copy->ncols = " << I_copy->ncols << std::endl;
    for (int i = 0; i < IDELEMS(I_copy); i++) {
        std::cout << "[DEBUG] I_copy[" << i << "] = " << p_String(I_copy->m[i], G.overpoly) << std::endl;
    }
    std::cout << "[DEBUG] Calling kStd..." << std::endl;
    I_std = kStd(I_copy, NULL, (tHomog)testHomog, NULL, NULL, 0, redSB);
    std::cout << "[DEBUG] kStd call complete" << std::endl;
    if (!I_std || !I_std->m) {
        std::cout << "[ERROR] Failed to compute valid standard basis" << std::endl;
        if (I_std) id_Delete(&I_std, G.overpoly);
        id_Delete(&I_copy, G.overpoly);
        G.over = NULL;
        G.overpoly = NULL;
        return G;
    }
    std::cout << "[DEBUG] Standard basis computed successfully" << std::endl;
    for (int i = 0; i < IDELEMS(I_std); i++) {
        std::cout << "[DEBUG] I_std[" << i << "] = " << p_String(I_std->m[i], G.overpoly) << std::endl;
    }
} catch (...) {
    std::cout << "[ERROR] Exception during standard basis computation" << std::endl;
    if (I_std) id_Delete(&I_std, G.overpoly);
    id_Delete(&I_copy, G.overpoly);
    G.over = NULL;
    G.overpoly = NULL;
    return G;
}

// Clean up the copy
id_Delete(&I_copy, G.overpoly);

std::cout << "[DEBUG] Standard basis computed successfully" << std::endl;

// Print the result
for (int i = 0; i < IDELEMS(I_std); i++) {
    if (!I_std->m[i]) continue;
    char* s = p_String(I_std->m[i], G.overpoly);
    std::cout << "[DEBUG] Standard basis element I_std[" << i << "] = " << s << std::endl;
    omFree(s);
}

// Cleanup
id_Delete(&I, G.over);

id_Delete(&I_mapped, G.overpoly);
id_Delete(&I_std, G.overpoly);

rChangeCurrRing(savedRing);
std::cout << "[DEBUG] Restored ring: " << rString(currRing) << std::endl;

    LabeledGraph G1 = G;
    lists eliminatedVars = (lists)omAlloc(sizeof(sleftv));
    eliminatedVars->Init(IDELEMS(I_std));

    // Iterate over standard basis elements
    for (int i = 0; i < IDELEMS(I_std); i++) {
        if (!I_std->m[i]) continue;
        poly p = I_std->m[i];

        // Find highest degree q variable
        int leadVar = -1;
        int maxDeg = 0;
        for (int j = 1; j <= 7; j++) { // Only check q(1) to q(7)
            int deg = p_GetExp(p, j, G.over);
            if (deg > maxDeg) {
                maxDeg = deg;
                leadVar = j;
            }
        }

        if (leadVar == -1 || maxDeg == 0) {
            std::cout << "[DEBUG] No q variables to eliminate in: " << pString(p) << std::endl;
            continue;
        }

        // Proceed with elimination
        std::cout << "[DEBUG] Eliminating q(" << leadVar << ") from: " << pString(p) << std::endl;
        
        // Get coefficient of leading term
        number coeff = n_Copy(p_GetCoeff(p, G.over), G.over->cf);
        if (!n_IsOne(coeff, G.over->cf)) {
            p = p_Mult_nn(p, n_Invers(coeff, G.over->cf), G.over);
            n_Delete(&coeff, G.over->cf);
        }

        poly head = p_Head(p, G.over);
        poly tail = p_Sub(p_Copy(p, G.over), p_Copy(head, G.over), G.over);
        poly rhs = p_Neg(p_Copy(tail, G.over), G.over);

        eliminatedVars->m[i].rtyp = POLY_CMD;
        eliminatedVars->m[i].data = p_Copy(head, G.over);
        G1 = substituteGraph(G1, head, rhs);

        // Clean up
        p_Delete(&head, G.over);
        p_Delete(&tail, G.over);
        p_Delete(&rhs, G.over);
    }

    G1.elimvars = eliminatedVars;
    std::cout << "[DEBUG] Elimination complete. Result:" << std::endl;
    printLabeledGraph(G1);
    id_Delete(&I_std, G.over);
    rChangeCurrRing(savedRing);
    return G1;
}




#include <singular/singular.h>

// Initialize rings
ring G_over = rDefault(0, 4, "p(1),p(2),p(3),p(4)", 7, "q(1),q(2),q(3),q(4),q(5),q(6),q(7)", "ip(7)");
ring G_overpoly = rDefault(QQ, 0, NULL, 11, "q(1),q(2),q(3),q(4),q(5),q(6),q(7),p(1),p(2),p(3),p(4)", "ip(11)");

// Set G_over as current ring to define the ideal
rChangeCurrRing(G_over);

// Define the balancing ideal I
ideal I = idInit(7, 1);
I->m[0] = p_Read("q(1)+q(4)+p(1)", G_over);
I->m[1] = p_Read("q(2)-q(5)+p(2)", G_over);
I->m[2] = p_Read("q(3)+q(6)+p(3)", G_over);
I->m[3] = p_Read("q(4)-q(7)+p(4)", G_over);
I->m[4] = p_Read("q(5)+p(1)-p(2)", G_over);
I->m[5] = p_Read("q(6)+p(2)-p(3)", G_over);
I->m[6] = p_Read("p(1)+p(2)+p(3)+p(4)", G_over);




void print_ideal(ideal I_mapped, ring r, const char* name) {
    rChangeCurrRing(r);
    std::cout << name << ":\n";
    for (int i = 0; i < IDELEMS(I_mapped); i++) {
        char* s = p_String(I_mapped->m[i], r);
        std::cout << "  I_mapped[" << i << "] = " << s << std::endl;
        omFree(s);
    }
    std::cout << std::endl;
}


// Switch to destination ring
rChangeCurrRing(G_overpoly);

// Create a new ideal in G.overpoly
ideal I_imap = idInit(IDELEMS(I), 1);

// Map each generator
for (int i = 0; i < IDELEMS(I); i++) {
    I_imap->m[i] = maIMap(G_over, G_overpoly, I->m[i]);
}

// Print the result
print_ideal(I_imap, G_overpoly, "I_imap");




rChangeCurrRing(G_overpoly);
ideal I_fetch = idInit(IDELEMS(I), 1);

for (int i = 0; i < IDELEMS(I); i++) {
    I_fetch->m[i] = prCopyR(I->m[i], G_over, G_overpoly);
}

print_ideal(I_fetch, G_overpoly, "I_fetch");



rChangeCurrRing(G_overpoly);

// Define the image of G.over variables in G.overpoly
ideal image_id = idInit(7, 1); // Only for q(1) to q(7)
for (int i = 0; i < 7; i++) {
    image_id->m[i] = p_ISet(1, G_overpoly);
    p_SetExp(image_id->m[i], i + 1, 1, G_overpoly); // Maps q(i) to q(i)
    p_Setm(image_id->m[i], G_overpoly);
}

// Define coefficient mapping
nMapFunc nMap = n_SetMap(G_over->cf, G_overpoly->cf);

// Map the ideal
ideal I_map = maMapIdeal(I, G_over, image_id, G_overpoly, nMap);

print_ideal(I_map, G_overpoly, "I_map");

// Clean up
idDelete(&image_id, G_overpoly);





rChangeCurrRing(G_overpoly);

// Permutation for variables: q(1) to q(1), ..., q(7) to q(7)
int perm[7] = {1, 2, 3, 4, 5, 6, 7};

// Permutation for parameters: p(1) to var8, ..., p(4) to var11
int par_perm[4] = {8, 9, 10, 11};

// Coefficient mapping
nMapFunc nMap = n_SetMap(G_over->cf, G_overpoly->cf);

// Map the ideal
ideal I_perm = id_PermIdeal(I, IDELEMS(I), 1, perm, G_over, G_overpoly, nMap, par_perm, 4, FALSE);

print_ideal(I_perm, G_overpoly, "I_perm");







rChangeCurrRing(G_overpoly);
ideal I_manual = idInit(IDELEMS(I), 1);

for (int i = 0; i < IDELEMS(I); i++) {
    char* s = p_String(I->m[i], G_over);
    std::string str(s);
    // Replace parameter notation, e.g., "(p(1))" to "p(1)"
    for (int j = 1; j <= 4; j++) {
        std::string old_pat = "(p(" + std::to_string(j) + "))";
        std::string new_pat = "p(" + std::to_string(j) + ")";
        size_t pos = 0;
        while ((pos = str.find(old_pat, pos)) != std::string::npos) {
            str.replace(pos, old_pat.length(), new_pat);
            pos += new_pat.length();
        }
    }
    poly p = NULL;
    p_Read(str.c_str(), &p, G_overpoly);
    I_manual->m[i] = p;
    omFree(s);
}

print_ideal(I_manual, G_overpoly, "I_manual");