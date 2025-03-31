intvec* deleteFromIntvec(const intvec* v, int j) {
    intvec* w = new intvec();
    for (int i = 0; i < v->length(); ++i) {
        if (i != j - 1) {
            (*w) += (*v)[i];
        }
    }
    return w;
}ring removeVariable(ring R, int j) {
    std::cout << "[DEBUG] ===== removeVariable called for index j = " << j << " =====" << std::endl;

    if (j < 1 || j > rVar(R)) {
        Werror("Index out of range in removeVariable: j = %d", j);
        return R;
    }

    lists L = rDecompose(R);
    lists varList = (lists)L->m[1].data;

    // Remove j-th variable
    for (int k = j - 1; k < varList->nr; ++k)
        varList->m[k] = varList->m[k + 1];
    varList->nr--;

    // Adjust ordering
    lists ordList = (lists)L->m[2].data;
    int nv = 0;
    for (int i = 0; i <= ordList->nr; ++i) {
        lists block = (lists)ordList->m[i].data;
        intvec* iv = (intvec*)block->m[1].data;
        nv += iv->length();
        if (nv >= j) {
            if (iv->length() == 1) {
                for (int k = i; k < ordList->nr; ++k)
                    ordList->m[k] = ordList->m[k + 1];
                ordList->nr--;
            } else {
                intvec* newiv = deleteFromIntvec(iv, j - nv + iv->length());
                block->m[1].data = (void*)newiv;
            }
            break;
        }
    }

    // Force coeff to QQ
    L->m[0].rtyp = INT_CMD;
    L->m[0].data = (void*)0;

    ring result = rCompose(L);
    if (!rTest(result)) {
        WerrorS("removeVariable: rCompose failed!");
        return NULL;
    }

    return result;
}

ring removeParameter(ring R, int j) {
    std::cout << "[DEBUG] ===== removeParameter called for index j = " << j << " =====" << std::endl;

    if (j < 1 || j > rPar(R)) {
        Werror("Index out of range in removeParameter: j = %d", j);
        return R;
    }

    lists L = rDecompose(R);
    lists parList = (lists)L->m[0].data;
    lists paramNames = (lists)parList->m[1].data;

    // Remove j-th parameter name
    for (int k = j - 1; k < paramNames->nr; ++k)
        paramNames->m[k] = paramNames->m[k + 1];
    paramNames->nr--;

    // Adjust order block
    lists ordList = (lists)L->m[2].data;
    int nv = 0;
    for (int i = 0; i <= ordList->nr; ++i) {
        lists block = (lists)ordList->m[i].data;
        intvec* iv = (intvec*)block->m[1].data;
        nv += iv->length();
        if (nv >= j) {
            if (iv->length() == 1) {
                for (int k = i; k < ordList->nr; ++k)
                    ordList->m[k] = ordList->m[k + 1];
                ordList->nr--;
            } else {
                intvec* newiv = deleteFromIntvec(iv, j - nv + iv->length());
                block->m[1].data = (void*)newiv;
            }
            break;
        }
    }

    ring result = rCompose(L);
    if (!rTest(result)) {
        WerrorS("removeParameter: rCompose failed!");
        return NULL;
    }

    return result;
}
LabeledGraph removeElimVars(LabeledGraph G) {
    std::cout << "******removing elimination variables in main.cpp******" << std::endl;
    std::cout << "[DEBUG] Starting removeElimVars" << std::endl;

    LabeledGraph G1;
    G1.vertices = G.vertices;
    G1.edges = G.edges;

    std::cout << "[DEBUG] G.overpoly = " << rString(G.overpoly) << std::endl;
    std::cout << "[DEBUG] G.over = " << rString(G.over) << std::endl;

    ring R1 = G.over;
    std::vector<int> iv; // indices of vars
    std::vector<int> ip; // indices of params

    for (int i = 0; i <= G.elimvars->nr; i++) {
        poly p = (poly)G.elimvars->m[i].Data();
        if (!p) continue;
        int varidx = rvar(p);
        if (varidx > 0) {
            iv.push_back(varidx);
        } else {
            for (int j = 1; j <= rPar(G.over); j++) {
                number coeff = pGetCoeff(p);
                if (n_IsParam(coeff, G.over)) {
                    ip.push_back(j);
                    break;
                }
            }
        }
    }

    std::sort(iv.begin(), iv.end(), std::greater<int>());
    std::sort(ip.begin(), ip.end(), std::greater<int>());

    for (int idx : iv) {
        std::cout << "[DEBUG] Removing variable: q(" << idx << ")" << std::endl;
        R1 = removeVariable(R1, idx);
    }

    for (int idx : ip) {
        std::cout << "[DEBUG] Removing parameter: p(" << idx << ")" << std::endl;
        R1 = removeParameter(R1, idx);
    }

    // Now handle overpoly
    ring RP1 = G.overpoly;
    lists elimvarsOver = (lists)omAlloc0(sizeof(slists));
    elimvarsOver->Init(G.elimvars->nr + 1);
    elimvarsOver->nr = G.elimvars->nr;

    for (int i = 0; i <= G.elimvars->nr; i++) {
        poly p = (poly)G.elimvars->m[i].Data();
        if (!p) continue;
        elimvarsOver->m[i].rtyp = POLY_CMD;
        elimvarsOver->m[i].data = (void*)imap(G.over, p); // map into overpoly ring
    }

    for (int i = 0; i <= elimvarsOver->nr; i++) {
        poly p = (poly)elimvarsOver->m[i].Data();
        if (!p) continue;
        int varidx = rvar(p);
        if (varidx > 0) {
            RP1 = removeVariable(RP1, varidx);
        }
    }

    // Map labels
    lists tr = (lists)omAlloc0(sizeof(slists));
    tr->Init(G.labels->nr + 1);
    tr->nr = G.labels->nr;
    for (int i = 0; i <= tr->nr; i++) {
        poly label = (poly)G.labels->m[i].Data();
        tr->m[i].Init();
        tr->m[i].rtyp = POLY_CMD;
        tr->m[i].data = (void*)imap(G.over, label);
    }

    G1.labels = tr;
    G1.over = R1;
    G1.overpoly = RP1;
    G1.elimvars = (lists)omAlloc0(sizeof(slists)); // empty
    G1.elimvars->Init(0);
    G1.elimvars->nr = -1;

    return G1;
}


#include <vector>
#include <string>
#include <algorithm>

// Helper function to remove an element from an intvec
static intvec* deleteFromIntvec(intvec* v, int j) {
    intvec* w = new intvec(v->length() - 1);
    int idx = 0;
    for (int i = 0; i < v->length(); i++) {
        if (i != j - 1) { // Singular uses 1-based indexing
            (*w)[idx++] = (*v)[i];
        }
    }
    return w;
}

// Remove a variable from a ring
ring removeVariable(ring R, int j) {
    if (j < 1 || j > rVar(R)) {
        Werror("Index out of range");
        return NULL;
    }

    lists L = rDecompose_list_cf(R);
    if (L == NULL) return NULL;

    // Delete the variable from the variable list
    lists vars = (lists)L->m[1].Data();
    vars->m[j-1].CleanUp(); // Clean up the deleted element
    for (int i = j; i <= vars->nr; i++) {
        vars->m[i-1] = vars->m[i];
    }
    vars->nr--;

    // Update the ordering blocks to remove the variable
    lists ords = (lists)L->m[2].Data();
    int nv = 0;
    for (int i = 0; i <= ords->nr; i++) {
        lists ordBlock = (lists)ords->m[i].Data();
        intvec* iv = (intvec*)ordBlock->m[1].Data();
        
        // Find the block containing our variable
        if (nv + iv->length() >= j) {
            if (iv->length() == 1) {
                // Remove the entire block
                ordBlock->CleanUp();
                for (int k = i; k < ords->nr; k++) {
                    ords->m[k] = ords->m[k+1];
                }
                ords->nr--;
            } else {
                // Remove just the variable from the block
                intvec* new_iv = deleteFromIntvec(iv, j - nv);
                ordBlock->m[1].rtyp = INTVEC_CMD;
                ordBlock->m[1].data = new_iv;
            }
            break;
        }
        nv += iv->length();
    }

    // Reconstruct the ring
    ring S = rCompose(L);
    L->Clean();
    return S;
}

// Remove a parameter from a ring
ring removeParameter(ring R, int j) {
    if (j < 1 || j > rPar(R)) {
        Werror("Index out of range");
        return NULL;
    }

    lists L = rDecompose_list_cf(R);
    if (L == NULL) return NULL;

    // Get the coefficient field description
    lists cf_desc = (lists)L->m[0].Data();
    
    // Delete the parameter from the parameter list
    lists params = (lists)cf_desc->m[1].Data();
    params->m[j-1].CleanUp(); // Clean up the deleted element
    for (int i = j; i <= params->nr; i++) {
        params->m[i-1] = params->m[i];
    }
    params->nr--;

    // Update the ordering blocks for parameters (if any)
    if (cf_desc->nr >= 2) { // Has ordering information
        lists ords = (lists)cf_desc->m[2].Data();
        int nv = 0;
        for (int i = 0; i <= ords->nr; i++) {
            lists ordBlock = (lists)ords->m[i].Data();
            intvec* iv = (intvec*)ordBlock->m[1].Data();
            
            // Find the block containing our parameter
            if (nv + iv->length() >= j) {
                if (iv->length() == 1) {
                    // Remove the entire block
                    ordBlock->CleanUp();
                    for (int k = i; k < ords->nr; k++) {
                        ords->m[k] = ords->m[k+1];
                    }
                    ords->nr--;
                } else {
                    // Remove just the parameter from the block
                    intvec* new_iv = deleteFromIntvec(iv, j - nv);
                    ordBlock->m[1].rtyp = INTVEC_CMD;
                    ordBlock->m[1].data = new_iv;
                }
                break;
            }
            nv += iv->length();
        }
    }

    // Reconstruct the ring
    ring S = rCompose(L);
    L->Clean();
    return S;
}

LabeledGraph removeElimVars(LabeledGraph G) {
    std::cout << "[DEBUG] Starting removeElimVars" << std::endl;
    
    ring R = G.over;
    std::cout << "[DEBUG] R = G.over = " << rString(R) << std::endl;
    
    ring RP = G.overpoly;
    std::cout << "[DEBUG] RP = G.overpoly = " << rString(RP) << std::endl;
    
    // Create new graph with same vertices and edges
    LabeledGraph G1;
    G1.vertices = G.vertices;
    G1.edges = G.edges;
    
    // Get elimination variables
    lists el = G.elimvars;
    std::cout << "[DEBUG] el = G.elimvars = " << lString(el) << std::endl;
    
    // Get labels
    lists lb = G.labels;
    std::cout << "[DEBUG] lb = G.labels = " << lString(lb) << std::endl;
    
    // Collect variables and parameters to remove
    std::vector<int> iv; // variables
    std::vector<int> ip; // parameters
    
    for (int i = 0; i <= el->nr; i++) {
        poly p = (poly)el->m[i].Data();
        if (p == NULL) continue;
        
        int var = p_IsPurePower(p, R);
        if (var > 0) {
            iv.push_back(var);
        } else {
            // Check if it's a parameter
            for (int j = 1; j <= rPar(R); j++) {
                poly par = p_Parameter(j, R);
                if (p_EqualPolys(p, par, R)) {
                    ip.push_back(j);
                    p_Delete(&par, R);
                    break;
                }
                p_Delete(&par, R);
            }
        }
    }
    
    // Sort in descending order for safe removal
    std::sort(iv.begin(), iv.end(), std::greater<int>());
    std::sort(ip.begin(), ip.end(), std::greater<int>());
    
    std::cout << "[DEBUG] iv = ";
    for (int v : iv) std::cout << v << " ";
    std::cout << std::endl;
    
    std::cout << "[DEBUG] ip = ";
    for (int p : ip) std::cout << p << " ";
    std::cout << std::endl;
    
    // Remove variables from R
    ring R1 = R;
    for (int v : iv) {
        std::cout << "[DEBUG] R1 before removing variable " << v << " = " << rString(R1) << std::endl;
        ring newR = removeVariable(R1, v);
        if (R1 != R) rDelete(R1); // Don't delete original ring
        R1 = newR;
        std::cout << "[DEBUG] R1 after removing variable " << v << " = " << rString(R1) << std::endl;
    }
    
    // Remove parameters from R
    for (int p : ip) {
        ring newR = removeParameter(R1, p);
        if (R1 != R) rDelete(R1); // Don't delete original ring
        R1 = newR;
        std::cout << "[DEBUG] R1 after removing parameter " << p << " = " << rString(R1) << std::endl;
    }
    
    // Remove variables from RP
    ring RP1 = RP;
    for (int v : iv) {
        std::cout << "[DEBUG] RP1 before removing variable " << v << " = " << rString(RP1) << std::endl;
        ring newRP = removeVariable(RP1, v);
        if (RP1 != RP) rDelete(RP1); // Don't delete original ring
        RP1 = newRP;
        std::cout << "[DEBUG] RP1 after removing variable " << v << " = " << rString(RP1) << std::endl;
    }
    
    // Map labels to new ring
    lists tr = (lists)omAllocBin(slists_bin);
    tr->Init(lb->nr + 1);
    for (int i = 0; i <= lb->nr; i++) {
        tr->m[i].rtyp = lb->m[i].rtyp;
        if (lb->m[i].rtyp == POLY_CMD && lb->m[i].Data() != NULL) {
            // Need to map the polynomial to the new ring
            // This is simplified - actual implementation would need proper mapping
            tr->m[i].data = p_Copy((poly)lb->m[i].Data(), R1);
        } else {
            tr->m[i].data = lb->m[i].CopyD();
        }
    }
    
    std::cout << "[DEBUG] tr = " << lString(tr) << std::endl;
    
    // Set up the new labeled graph
    G1.labels = tr;
    G1.over = R1;
    G1.overpoly = RP1;
    G1.elimvars = NULL;
    
    return G1;
}
// Forward declarations (assumed to exist in the codebase)
ring removeVariable(ring R, int j);
ring removeParameter(ring R, int j);
void printLabeledGraph(const LabeledGraph& G);

// Procedure to remove elimination variables from a labeled graph
LabeledGraph removeElimVars(const LabeledGraph& G) {
    std::cout << "[DEBUG] Starting removeElimVars" << std::endl;

    // Create a new labeled graph to store the result
    LabeledGraph G1;

    // Copy vertices and edges from the input graph
    G1.vertices = G.vertices;
    G1.edges = G.edges;

    // Get the base ring and polynomial ring
    ring R = G.over;
    ring RP = G.overpoly;
    std::cout << "[DEBUG] R = G.over = " << rString(R) << std::endl;
    std::cout << "[DEBUG] RP = G.overpoly = " << rString(RP) << std::endl;

    // Switch to the base ring
    ring savedRing = currRing;
    rChangeCurrRing(R);

    // Get elimination variables and labels
    lists el = G.elimvars;
    lists lb = G.labels;
    std::cout << "[DEBUG] el = G.elimvars = ";
    printListAsString("el", el);
    std::cout << "[DEBUG] lb = G.labels = ";
    printListAsString("lb", lb);

    // Lists to store variable and parameter indices to remove
    std::vector<int> iv; // variable indices
    std::vector<int> ip; // parameter indices

    // Process each elimination variable
    if (el) {
        for (int i = 0; i <= el->nr; ++i) {
            poly elim_var = (poly)el->m[i].Data();
            if (!elim_var) continue;

            int var_idx = p_Var(elim_var, R);
            if (var_idx != 0) {
                std::cout << "[DEBUG] rvar(el[" << i << "]) = " << var_idx << std::endl;
                iv.push_back(var_idx);
            } else {
                std::cout << "[DEBUG] el[" << i << "] = ";
                char* str = p_String(elim_var, R);
                std::cout << str << std::endl;
                omFree(str);

                for (int j = 0; j < rPar(R); ++j) {
                    char* param = rParameter(R)[j];
                    poly param_poly = p_ISet(1, R); // Dummy polynomial for comparison
                    if (p_ComparePolys(elim_var, param_poly, R)) {
                        ip.push_back(j + 1); // 1-based index
                        break;
                    }
                    p_Delete(&param_poly, R);
                }
            }
        }
    }

    // Sort indices in ascending order
    std::sort(iv.begin(), iv.end());
    std::sort(ip.begin(), ip.end());
    std::cout << "[DEBUG] iv = [";
    for (size_t i = 0; i < iv.size(); ++i) {
        std::cout << iv[i];
        if (i < iv.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << "[DEBUG] ip = [";
    for (size_t i = 0; i < ip.size(); ++i) {
        std::cout << ip[i];
        if (i < ip.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    // Remove variables from R
    ring R1 = R;
    for (int i = iv.size() - 1; i >= 0; --i) {
        R1 = removeVariable(R1, iv[i]);
        std::cout << "[DEBUG] R1 after removing variable " << iv[i] << " = " << rString(R1) << std::endl;
    }

    // Remove parameters from R
    for (int i = ip.size() - 1; i >= 0; --i) {
        R1 = removeParameter(R1, ip[i]);
        std::cout << "[DEBUG] R1 after removing parameter " << ip[i] << " = " << rString(R1) << std::endl;
    }

    // Switch to the polynomial ring
    rChangeCurrRing(RP);
    lists el_mapped = (lists)omAlloc0(sizeof(slists));
    el_mapped->Init(el ? el->nr + 1 : 0);
    el_mapped->nr = el ? el->nr : -1;

    // Map elimination variables to RP and collect variable indices
    std::vector<int> iv_RP;
    if (el) {
        nMapFunc nMap = n_SetMap(R->cf, RP->cf);
        int nvars_R = rVar(R);
        int npars_R = rPar(R);
        int* perm = (int*)omAlloc0((nvars_R + 1) * sizeof(int));
        int* par_perm = (int*)omAlloc0((npars_R + 1) * sizeof(int));
        for (int i = 1; i <= nvars_R; ++i) perm[i] = i + npars_R;
        for (int i = 0; i < npars_R; ++i) par_perm[i] = i + 1;

        for (int i = 0; i <= el->nr; ++i) {
            poly p = (poly)el->m[i].Data();
            if (p) {
                poly p_mapped = p_PermPoly(p, perm, R, RP, nMap, par_perm, npars_R, FALSE);
                el_mapped->m[i].rtyp = POLY_CMD;
                el_mapped->m[i].data = p_mapped;
                std::cout << "[DEBUG] el[" << i << "] = ";
                char* str = p_String(p_mapped, RP);
                std::cout << str << std::endl;
                omFree(str);

                int var_idx = p_Var(p_mapped, RP);
                if (var_idx != 0) iv_RP.push_back(var_idx);
            }
        }
        omFree(perm);
        omFree(par_perm);
    }

    // Sort variable indices for RP
    std::sort(iv_RP.begin(), iv_RP.end());
    std::cout << "[DEBUG] iv_RP = [";
    for (size_t i = 0; i < iv_RP.size(); ++i) {
        std::cout << iv_RP[i];
        if (i < iv_RP.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    // Remove variables from RP
    ring RP1 = RP;
    for (int i = iv_RP.size() - 1; i >= 0; --i) {
        std::cout << "[DEBUG] RP1 before removing variable " << iv_RP[i] << " = " << rString(RP1) << std::endl;
        RP1 = removeVariable(RP1, iv_RP[i]);
        std::cout << "[DEBUG] RP1 after removing variable " << iv_RP[i] << " = " << rString(RP1) << std::endl;
    }

    // Switch back to R1 and map labels
    rChangeCurrRing(R1);
    lists tr = (lists)omAlloc0(sizeof(slists));
    tr->Init(lb ? lb->nr + 1 : 0);
    tr->nr = lb ? lb->nr : -1;

    if (lb) {
        nMapFunc nMap = n_SetMap(R->cf, R1->cf);
        int nvars_R = rVar(R);
        int npars_R = rPar(R);
        int* perm = (int*)omAlloc0((nvars_R + 1) * sizeof(int));
        int* par_perm = (int*)omAlloc0((npars_R + 1) * sizeof(int));
        for (int i = 1; i <= nvars_R; ++i) perm[i] = i; // Simplified mapping
        for (int i = 0; i < npars_R; ++i) par_perm[i] = i + 1;

        for (int i = 0; i <= lb->nr; ++i) {
            poly p = (poly)lb->m[i].Data();
            if (p) {
                poly p_mapped = p_PermPoly(p, perm, R, R1, nMap, par_perm, npars_R, FALSE);
                tr->m[i].rtyp = POLY_CMD;
                tr->m[i].data = p_mapped;
            }
        }
        omFree(perm);
        omFree(par_perm);
    }
    std::cout << "[DEBUG] tr = ";
    printListAsString("tr", tr);

    // Set up the resulting graph
    G1.labels = tr;
    G1.over = R1;
    G1.overpoly = RP1;
    G1.elimvars = (lists)omAlloc0(sizeof(slists)); // Empty list
    G1.elimvars->Init(0);
    G1.elimvars->nr = -1;

    // Restore original ring
    rChangeCurrRing(savedRing);
    return G1;
}

// Supporting function: Remove a variable from a ring
ring removeVariable(ring R, int j) {
    if (j < 1 || j > rVar(R)) {
        std::cerr << "Error: Index out of range" << std::endl;
        return R;
    }

    lists L = rDecompose(R);
    lists vars = (lists)L->m[1].Data();
    lists orders = (lists)L->m[2].Data();

    // Remove the j-th variable
    lists new_vars = (lists)omAlloc0(sizeof(slists));
    new_vars->Init(vars->nr);
    new_vars->nr = vars->nr - 1;
    int idx = 0;
    for (int i = 0; i <= vars->nr; ++i) {
        if (i + 1 != j) {
            new_vars->m[idx] = vars->m[i];
            vars->m[i].data = nullptr; // Prevent double freeing
            idx++;
        } else {
            omFree(vars->m[i].data);
        }
    }

    // Adjust the ordering (simplified assumption: single ordering block)
    int total_vars = 0;
    for (int i = 0; i <= orders->nr; ++i) {
        lists order = (lists)orders->m[i].Data();
        intvec* iv = (intvec*)order->m[1].Data();
        total_vars += iv->length();
        if (total_vars >= j) {
            intvec* new_iv = new intvec(iv->length() - 1);
            int new_idx = 0;
            for (int k = 0; k < iv->length(); ++k) {
                if (k + 1 != (j - (total_vars - iv->length()))) {
                    (*new_iv)[new_idx++] = (*iv)[k];
                }
            }
            delete iv;
            order->m[1].data = new_iv;
            break;
        }
    }

    L->m[1].data = new_vars;
    ring S = rCompose(L); // Assuming rCompose exists to reconstruct the ring
    rDelete(R);
    omFreeBin(L, slists_bin);
    return S;
}

// Supporting function: Remove a parameter from a ring
ring removeParameter(ring R, int j) {
    if (j < 1 || j > rPar(R)) {
        std::cerr << "Error: Index out of range" << std::endl;
        return R;
    }

    lists L = rDecompose(R);
    coeffs cf = (coeffs)L->m[0].Data();
    char** params = rParameter(R);
    int npars = rPar(R);

    // Create new parameter list
    char** new_params = (char**)omAlloc0((npars - 1) * sizeof(char*));
    int idx = 0;
    for (int i = 0; i < npars; ++i) {
        if (i + 1 != j) {
            new_params[idx++] = omStrDup(params[i]);
        }
    }

    // Update coefficient ring (simplified for common cases)
    if (rField_is_Zp(R) || rField_is_Q(R)) {
        L->m[0].data = (void*)(long)cf->ch;
    } else {
        // More complex coefficient rings would need specific handling
        cf->ref--;
        nKillChar(cf);
        cf = nInitChar(n_Zp, (void*)(long)cf->ch); // Example fallback
        L->m[0].data = cf;
    }
    rSetParameters(R, new_params, npars - 1);

    ring S = rCompose(L);
    rDelete(R);
    omFreeBin(L, slists_bin);
    for (int i = 0; i < npars - 1; ++i) omFree(new_params[i]);
    omFree(new_params);
    return S;
}
