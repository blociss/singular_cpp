#include "feynman.h"
#include <iostream>
#include <algorithm>  // For std::sort
#include <singular/Singular/libsingular.h>
#include "polys/ext_fields/transext.h"
#include <singular/polys/monomials/ring.h>
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/polys/simpleideals.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/prCopy.h>
#include <kernel/combinatorics/stairc.h> // For scKBase
#include <sstream>
#include <kernel/groebner_walk/walkSupport.h>

// Convert ideal to module with gen(i) components
// Convert ideal to module with gen(i) components


// Print an ideal for debugging
void printIdeal(const ideal I)
{
    if (!I) {
        std::cout << "Ideal is NULL" << std::endl;
        return;
    }
    
    ring savedRing = currRing;
    
    std::cout << "Ideal with " << IDELEMS(I) << " elements:" << std::endl;
    for (int i = 0; i < IDELEMS(I); i++) {
        poly p = I->m[i];
        if (p) {
            char* pStr = p_String(p, currRing);
            std::cout  << (pStr ? pStr : "null") << ",";
            if (pStr) omFree(pStr);
        }
        else {
            std::cout << " 0" << ",";
        }
    }
    std::cout<<std::endl;
    rChangeCurrRing(savedRing);
}

    ring createRing(char **extNames, int extCount, char **varNames, int varCount, rRingOrder_t varOrdering) {
        // Create base coefficient field
        std::cout << "[DEBUG] Creating base coefficient field" << std::endl;
        coeffs cf = nInitChar(n_Q, NULL);
        if (!cf) return NULL;

        // Create extension field if needed
        ring extRing = NULL;
        if (extCount > 0) {
            // Create extension ring with default ordering
            extRing = rDefault(cf, extCount, extNames);
            if (!extRing) {
                nKillChar(cf);
                return NULL;
            }

            // Create extension coefficient field
            TransExtInfo extParam;
            extParam.r = extRing;
            coeffs extCf = nInitChar(n_transExt, &extParam);

            // Clean up and check
            rKill(extRing);
            nKillChar(cf);
            
            if (!extCf) return NULL;
            cf = extCf;
        }
    
        // Create variable names array
        char **vars = (char **)omAlloc0(varCount * sizeof(char *));
        if (!vars) {
            nKillChar(cf);
            return NULL;
        }

        // Copy variable names
        for (int i = 0; i < varCount; ++i) {
            vars[i] = omStrDup(varNames[i]);
            if (!vars[i]) {
                for (int j = 0; j < i; ++j) omFree(vars[j]);
                omFree(vars);
                nKillChar(cf);
                return NULL;
            }
        }
    
        // Create ring with specified ordering
        ring R = rDefault(cf, varCount, vars, varOrdering);
        // Clean up variable names
        for (int i = 0; i < varCount; ++i) omFree(vars[i]);
        omFree(vars);

        if (!R) {
            nKillChar(cf);
            return NULL;
        }

        // Initialize ring
        rComplete(R);

        return R;
    }

// Helper function to get size of a list
int size(lists L) {
    if (!L) return 0;
    return L->nr + 1;
}



// Helper function to create an empty string list of given width
lists emptyString(int width) {
    lists result = (lists)omAlloc(sizeof(sleftv));
    result->Init(1);
    char* str = (char*)omAlloc(width + 1);
    memset(str, ' ', width);
    str[width] = '\0';
    result->m[0].rtyp = STRING_CMD;
    result->m[0].data = (void*)str;
    result->nr = 0;
    return result;
}

// netList: Convert list to Net with delimiters
Net netList(lists L1) {
    Net N;
    if (!L1) {
        N.rows = NULL;
        return N;
    }
    
    // Allocate memory for N.rows
    N.rows = (lists)omAllocBin(slists_bin);
    if (!N.rows) {
        std::cerr << "Memory allocation failed for N.rows\n";
        exit(1);
    }
    N.rows->Init();
    N.rows->m = (leftv)omAlloc0((L1->nr + 1) * sizeof(sleftv));
    N.rows->nr = L1->nr;
    
    // Copy each element from L1 to N.rows
    for (int i = 0; i <= L1->nr; i++) {
        N.rows->m[i].Init();
        N.rows->m[i].rtyp = L1->m[i].rtyp;
        if (L1->m[i].rtyp == LIST_CMD) {
            lists subList = (lists)L1->m[i].Data();
            if (subList) {
                lists newSubList = (lists)omAllocBin(slists_bin);
                newSubList->Init();
                newSubList->m = (leftv)omAlloc0((subList->nr + 1) * sizeof(sleftv));
                newSubList->nr = subList->nr;
                
                for (int j = 0; j <= subList->nr; j++) {
                    newSubList->m[j].Init();
                    newSubList->m[j].rtyp = subList->m[j].rtyp;
                    newSubList->m[j].data = subList->m[j].data;
                }
                N.rows->m[i].data = newSubList;
            }
        } else {
            // Deep copy the data for non-list types
            if (L1->m[i].rtyp == INT_CMD) {
                N.rows->m[i].data = (void*)((long)L1->m[i].data);
            } else if (L1->m[i].rtyp == STRING_CMD) {
                const char* str = (const char*)L1->m[i].data;
                if (str) {
                    N.rows->m[i].data = omStrDup(str);
                } else {
                    N.rows->m[i].data = NULL;
                }
            } else {
                N.rows->m[i].data = NULL;  // For other types, we don't copy for now
            }
        }
    }
    
    return N;
}


// makeGraph: Create a graph from vertex and edge lists
Graph makeGraph(lists vertices, lists edges) {
    std::cout << "[DEBUG] Entering makeGraph\n";
    Graph G;
    G.vertices = vertices;
    G.edges = edges;
    std::cout << "[DEBUG] Exiting makeGraph\n";
    return G;
}
// makeLabeledGraph: Safely create a labeled graph with labels correctly initialized in ring R
LabeledGraph makeLabeledGraph(lists vertices, lists edges, ring R, lists labels, ring Rpoly) {
    //std::cout << "[DEBUG] Entering makeLabeledGraph\n";    LabeledGraph G;
    LabeledGraph G;
    G.vertices = vertices;
    G.edges = edges;

    // Save the original ring context
    ring savedRing = currRing;
    // Set ring R as current and copy it
    rChangeCurrRing(R);
    G.over = rCopy(R);

    // Handle labels
    if (labels != NULL) {
        // Allocate labels safely in ring R
        G.labels = (lists)omAllocBin(slists_bin);
        G.labels->Init(labels->nr + 1);
        for (int i = 0; i <= labels->nr; i++) {
            G.labels->m[i].Init();
            G.labels->m[i].rtyp = labels->m[i].rtyp;
            if (labels->m[i].rtyp == POLY_CMD) {
                // Critical: labels must be created or mapped within the current ring context (R)
                G.labels->m[i].data = p_Copy((poly)labels->m[i].data, R);
            } else {
                G.labels->m[i].data = labels->m[i].data;
            }
        }
    } else {
        G.labels = NULL;
    }

    // Set overpoly as provided (Rpoly), typically the polynomial ring P
    G.overpoly = rCopy(Rpoly);

    // Initialize optional fields as null
    G.elimvars = NULL;
    G.baikovover = NULL;
    G.baikovmatrix = NULL;

    // Restore the original ring
    rChangeCurrRing(savedRing);

    return G;
}

// printGraph: Print graph details
void printGraph(const Graph& G) {
    std::cout << "[DEBUG] Entering printGraph\n";
    Net edgeNet = netList(G.edges);
    printNet(edgeNet);

    int ct = 0;
    int edgesSize = G.edges ? G.edges->nr + 1 : 0;
    for (int i = 0; i < edgesSize; i++) {
        lists edge = (lists)G.edges->m[i].Data();
        int edgeSize = edge ? edge->nr + 1 : 0;
        if (edgeSize == 1) ct++;
    }

    std::string msg;
    if (ct != 0) {
        msg = "Graph with " + std::to_string(G.vertices ? G.vertices->nr + 1 : 0) + 
              " vertices, " + std::to_string(edgesSize - ct) + 
              " bounded edges and " + std::to_string(ct) + " unbounded edges";
    } else {
        msg = "Graph with " + std::to_string(G.vertices ? G.vertices->nr + 1 : 0) + 
              " vertices and " + std::to_string(edgesSize) + " edges";
    }
    std::cout << msg << std::endl;

    // Clean up the Net structure
    if (edgeNet.rows) {
        for (int i = 0; i <= edgeNet.rows->nr; i++) {
            if (edgeNet.rows->m[i].rtyp == LIST_CMD && edgeNet.rows->m[i].data) {
                lists subList = (lists)edgeNet.rows->m[i].data;
                omFreeBin(subList->m, (subList->nr + 1) * sizeof(sleftv));
                omFreeBin(subList, slists_bin);
            }
        }
        omFreeBin(edgeNet.rows->m, (edgeNet.rows->nr + 1) * sizeof(sleftv));
        omFreeBin(edgeNet.rows, slists_bin);
    }
    std::cout << "[DEBUG] Exiting printGraph\n";
}

// Helper to create an integer list
lists createIntList(int* values, int size) {
    std::cout << "[DEBUG] Entering createIntList\n";
    lists L = (lists)omAlloc(sizeof(sleftv));
    L->Init(size);
    for (int i = 0; i < size; i++) {
        L->m[i].rtyp = INT_CMD;
        L->m[i].data = (void*)(long)values[i];
    }
    L->nr = size - 1;
    std::cout << "[DEBUG] Exiting createIntList\n";
    return L;
}

// Helper to create a list of edge lists
lists createEdgeList(int edges[][2], int size, int singleEdges[], int singleSize) {
    std::cout << "[DEBUG] Entering createEdgeList with " << size << " bounded and " << singleSize << " unbounded edges" << std::endl;
    
    lists L = (lists)omAlloc(sizeof(sleftv));
    if (!L) {
        std::cout << "[ERROR] Failed to allocate main list" << std::endl;
        return NULL;
    }
    L->Init(size + singleSize);

    // Create bounded edges
    for (int i = 0; i < size; i++) {
        lists edge = (lists)omAlloc(sizeof(sleftv));
        if (!edge) {
            std::cout << "[ERROR] Failed to allocate bounded edge " << i << std::endl;
            // Clean up previously created edges
            for (int j = 0; j < i; j++) {
                lists prevEdge = (lists)L->m[j].data;
                if (prevEdge) {
                    omFree(prevEdge);
                }
            }
            omFree(L);
            return NULL;
        }
        edge->Init(2);
        edge->m[0].rtyp = INT_CMD;
        edge->m[0].data = (void*)(long)edges[i][0];
        edge->m[1].rtyp = INT_CMD;
        edge->m[1].data = (void*)(long)edges[i][1];
        edge->nr = 1;
        L->m[i].rtyp = LIST_CMD;
        L->m[i].data = (void*)edge;
    }

    // Create unbounded edges
    for (int i = 0; i < singleSize; i++) {
        lists edge = (lists)omAlloc(sizeof(sleftv));
        if (!edge) {
            std::cout << "[ERROR] Failed to allocate unbounded edge " << i << std::endl;
            // Clean up all previously created edges
            for (int j = 0; j < size + i; j++) {
                lists prevEdge = (lists)L->m[j].data;
                if (prevEdge) {
                    omFree(prevEdge);
                }
            }
            omFree(L);
            return NULL;
        }
        edge->Init(1);
        edge->m[0].rtyp = INT_CMD;
        edge->m[0].data = (void*)(long)singleEdges[i];
        edge->nr = 0;
        L->m[size + i].rtyp = LIST_CMD;
        L->m[size + i].data = (void*)edge;
    }
    
    L->nr = size + singleSize - 1;
    std::cout << "[DEBUG] Successfully created edge list" << std::endl;
    return L;
}
Net net(const std::string& str) {
    Net N;
    N.rows = (lists)omAlloc(sizeof(slists));
    N.rows->Init(1);
    N.rows->m[0].rtyp = STRING_CMD;
    N.rows->m[0].data = omStrDup(str.c_str());
    return N;
}


// printNet: Print the rows of a Net
void printNet(const Net& N) {
    if (!N.rows) {
        std::cout << "[]" << std::endl;
        return;
    }
    
    std::cout << "[";
    for (int i = 0; i <= N.rows->nr; i++) {
        if (N.rows->m[i].rtyp == LIST_CMD) {
            lists subList = (lists)N.rows->m[i].Data();
            std::cout << "[";
            for (int j = 0; j <= subList->nr; j++) {
                if (subList->m[j].rtyp == INT_CMD) {
                    std::cout << (long)subList->m[j].Data();
                    if (j < subList->nr) std::cout << ",";
                }
            }
            std::cout << "]";
        } else if (N.rows->m[i].rtyp == STRING_CMD) {
            std::cout << (char*)N.rows->m[i].Data();
        }
        if (i < N.rows->nr) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

// printLabeledGraph: Print the labeled graph with edges and edge terms
void printLabeledGraph(const LabeledGraph& G) {
    // Print the edges as a Net
    Net edgeNet = netList(G.edges);
    printNet(edgeNet);

    // Count unbounded edges (edges with a single vertex)
    int ct = 0;
    int edgesSize = size(G.edges);
    for (int i = 1; i <= edgesSize; i++) {
        lists edge = (lists)G.edges->m[i - 1].Data();
        if (size(edge) == 1) {
            ct++;
        }
    }

    // Print graph summary
    int vertexCount = size(G.vertices);
    if (ct != 0) {
        std::cout << "Graph with " << vertexCount << " vertices, "
                  << (edgesSize - ct) << " bounded edges and "
                  << ct << " unbounded edges" << std::endl;
    } else {
        std::cout << "Graph with " << vertexCount << " vertices and "
                  << edgesSize << " edges" << std::endl;
    }

    // Save the current ring
    ring savedRing = currRing;
    
    // Switch to the graph's ring
    if (G.over == nullptr) {
        std::cerr << "[ERROR] Graph ring is null" << std::endl;
        return;
    }
    rChangeCurrRing(G.over);
    
    // Get the labels list
    lists labels = G.labels;
    if (labels == nullptr) {
        std::cerr << "[ERROR] Labels list is null" << std::endl;
        rChangeCurrRing(savedRing);
        return;
    }

    // Print "Edgeterms:" header
    std::cout << std::endl << "Edgeterms: " << std::endl;

    // Create a list of strings for each edge term
    lists ev = (lists)omAllocBin(slists_bin);
    if (!ev) {
        std::cerr << "[ERROR] Failed to allocate memory for edge list" << std::endl;
        rChangeCurrRing(savedRing);
        return;
    }
    ev->Init();
    ev->m = (leftv)omAlloc0(edgesSize * sizeof(sleftv));
    ev->nr = edgesSize - 1;
    for (int i = 0; i < edgesSize; i++) {
        ev->m[i].Init();
    }

    // Build edge terms
    for (int i = 1; i <= edgesSize; i++) {
        // Construct edge string
        lists edgeData = (lists)G.edges->m[i - 1].Data();
        std::string edgeStr;
        if (size(edgeData) == 2) {
            int v1 = (int)(long)edgeData->m[0].Data();
            int v2 = (int)(long)edgeData->m[1].Data();
            edgeStr = "[" + std::to_string(v1) + "," + std::to_string(v2) + "]";
        } else if (size(edgeData) == 1) {
            int v = (int)(long)edgeData->m[0].Data();
            edgeStr = "[" + std::to_string(v) + "]";
        } else {
            edgeStr = "[]";
        }

        // Construct label string
        std::string labelStr = "NULL";
        if (labels && i <= labels->nr + 1 && labels->m[i - 1].rtyp == POLY_CMD) {
            poly labelPoly = (poly)labels->m[i - 1].Data();
            if (labelPoly) {
                char* polyStr = p_String(labelPoly, G.over);
                if (polyStr) {
                    // Keep the original polynomial string representation
                    labelStr = std::string(polyStr);
                    omFree(polyStr);
                }
            }
        }

        // Combine edge and label into a single string
        std::string edgeTerm = edgeStr + " => " + labelStr;

        // Store in ev
        ev->m[i - 1].rtyp = STRING_CMD;
        ev->m[i - 1].data = omStrDup(edgeTerm.c_str());
    }

    // Print the list of edge terms
    Net evNet = netList(ev);
    printNet(evNet);

    // Clean up
    if (ev) {
        for (int i = 0; i < edgesSize; i++) {
            if (ev->m[i].data) {
                omFree(ev->m[i].data);
            }
        }
        omFree(ev->m);
        omFreeBin(ev, slists_bin);
    }
    rChangeCurrRing(savedRing);
}


// Helper function to concatenate Nets
Net catNet(const Net& N, const Net& M) {
    std::cout << "[DEBUG] Entering catNet" << std::endl;
    
    // Create result Net
    Net NM;
    NM.rows = (lists)omAllocBin(slists_bin);
    if (!NM.rows) {
        std::cerr << "[ERROR] Failed to allocate memory for result rows" << std::endl;
        return NM;
    }
    
    // Get input lists
    lists LN = N.rows;
    lists LM = M.rows;
    
    std::cout << "[DEBUG] Input nets: "
              << "N=" << (LN ? "present" : "null") << ", "
              << "M=" << (LM ? "present" : "null") << std::endl;
    
    // Initialize result list with size 1 since we're concatenating strings
    NM.rows->Init(1);
    NM.rows->m[0].Init();
    NM.rows->m[0].rtyp = STRING_CMD;
    
    // Get strings from both Nets
    std::string result;
    
    if (LN && LN->m[0].rtyp == STRING_CMD) {
        char* strN = (char*)LN->m[0].Data();
        if (strN) {
            result += strN;
            std::cout << "[DEBUG] First string: " << strN << std::endl;
        }
    }
    
    if (LM && LM->m[0].rtyp == STRING_CMD) {
        char* strM = (char*)LM->m[0].Data();
        if (strM) {
            result += strM;
            std::cout << "[DEBUG] Second string: " << strM << std::endl;
        }
    }
    
    std::cout << "[DEBUG] Concatenated result: " << result << std::endl;
    
    // Store result
    NM.rows->m[0].data = omStrDup(result.c_str());
    
    std::cout << "[DEBUG] Exiting catNet" << std::endl;
    return NM;
}
void printMatrix(const matrix m)
{
  int rr = MATROWS(m); int cc = MATCOLS(m);
  printf("\n-------------\n");
  for (int r = 1; r <= rr; r++)
  {
    for (int c = 1; c <= cc; c++)
      printf("%s  ", pString(MATELEM(m, r, c)));
    printf("\n");
  }
  printf("-------------\n");
}

// Create a labeled graph from vertices and edges
LabeledGraph makeLabeledGraph(lists vertices, lists edges) {
    LabeledGraph G;
    G.vertices = vertices;
    G.edges = edges;
    G.over = currRing;
    G.overpoly = NULL;
    G.elimvars = NULL;
    G.baikovover = NULL;
    G.baikovmatrix = NULL;
    return G;
}

void printElimVarsOnly(const lists elimvars, ring r) {
    if (!elimvars) {
        std::cout << "[DEBUG] elimvars is NULL" << std::endl;
        return;
    }

    ring savedRing = currRing;
    rChangeCurrRing(r);

   // std::cout << "\nElimination Variables:\n";

    int nr = elimvars->nr;
    for (int i = 0; i <= nr; ++i) {
        sleftv* lv = &(elimvars->m[i]);

       // std::cout << "[DEBUG] elimvar " << i << ": ";

        if (lv->rtyp == POLY_CMD && lv->data != nullptr) {
            poly p = (poly)(lv->data);
            char* str = p_String(p, r);
            if (str) {
                std::cout << str << std::endl;
                omFree(str);
            } else {
               std::cout << "(conversion failed)" << std::endl;
            }
        } else if (lv->data == nullptr) {
            //std::cout << "NULL polynomial" << std::endl;
        } else {
            std::cout << "Non-polynomial or garbage data (rtyp=" << lv->rtyp << ")" << std::endl;
        }
    }

    rChangeCurrRing(savedRing);
}
void printListAsString(const char* name, lists L, bool typed = false, int dim = 1) {
    if (!L) {
        std::cout << "[DEBUG] " << name << " = <NULL>" << std::endl;
        return;
    }

    char* listStr = lString(L, typed, dim);
    std::cout << "[DEBUG] " << name << " = " << listStr << std::endl;
    omFree(listStr);
}
void printPolyListAsVector(const char* name, lists L, ring r) {
    if (!L) {
        std::cout << "[DEBUG] " << name << " = <NULL>" << std::endl;
        return;
    }

    ring saved = currRing;
    rChangeCurrRing(r);

    std::cout << "[DEBUG] " << name << " = [";
    for (int i = 0; i <= L->nr; i++) {
        if (L->m[i].rtyp == POLY_CMD && L->m[i].data != nullptr) {
            poly p = (poly)L->m[i].data;
            char* str = p_String(p, r);
            std::cout << str;
            omFree(str);
        } else {
            std::cout << "NULL";
        }
        if (i < L->nr) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    rChangeCurrRing(saved);
}

// Helper function to delete an element from an intvec
intvec* deleteFromIntvec(intvec* iv, int pos) {
    if (pos < 1 || pos > iv->length()) return iv;
    intvec* newiv = new intvec(iv->length() - 1);
    int k = 0;
    for (int i = 0; i < iv->length(); ++i) {
        if (i + 1 != pos) (*newiv)[k++] = (*iv)[i];
    }
    delete iv;
    return newiv;
}
ring removeVariable(ring R, int j) {
    //std::cout << "[DEBUG] ===== removeVariable called for index j = " << j << " =====" << std::endl;
   // std::cout << "[DEBUG] Input ring R: " << rString(R) << std::endl;
    //std::cout << "[DEBUG] Input ring coefficient field type: " << nCoeffName(R->cf) << std::endl;

    if (j < 1 || j > rVar(R)) {
        Werror("Index out of range in removeVariable: j = %d, nvars = %d", j, rVar(R));
        return R;
    }

    lists L = rDecompose(R);
    if (!L) {
        WerrorS("Failed to decompose ring");
        return R;
    }

    // Remove j-th variable from varList
    lists varList = (lists)L->m[1].data;
    omFree(varList->m[j - 1].data); // Free the variable name
    for (int k = j - 1; k < varList->nr; ++k) {
        varList->m[k] = varList->m[k + 1];
    }
    varList->nr--;

    // Adjust ordering blocks
    lists ordList = (lists)L->m[2].data;
    int nv = 0;
    int new_var_count = rVar(R) - 1;
    for (int i = 0; i <= ordList->nr; ++i) {
        lists block = (lists)ordList->m[i].data;
        intvec* iv = (intvec*)block->m[1].data;
        nv += iv->length();
        if (nv >= j) {
            if (iv->length() == 1) {
                omFree(block->m[0].data);
                delete iv;
                omFreeBin(block, slists_bin);
                for (int k = i; k < ordList->nr; ++k) {
                    ordList->m[k] = ordList->m[k + 1];
                }
                ordList->nr--;
            } else {
                intvec* newiv = deleteFromIntvec(iv, j - (nv - iv->length()));
                block->m[1].data = (void*)newiv;
            }
            break;
        }
    }

    // Ensure ordList has exactly one ip block
    if (ordList->nr > 0) {
        for (int i = 1; i <= ordList->nr; ++i) {
            lists block = (lists)ordList->m[i].data;
            if (block) {
                omFree(block->m[0].data);
                delete (intvec*)block->m[1].data;
                omFreeBin(block, slists_bin);
            }
        }
        ordList->nr = 0; // Keep only the first ip block
    }

    // Verify and fix the ip block
    lists ip_block = (lists)ordList->m[0].data;
    intvec* iv = (intvec*)ip_block->m[1].data;
    if (iv->length() != new_var_count) {
        delete iv;
        iv = new intvec(new_var_count);
        for (int k = 0; k < new_var_count; ++k) (*iv)[k] = 1; // Weights = 1
        ip_block->m[1].data = (void*)iv;
    }
    if (strcmp((char*)ip_block->m[0].data, "ip") != 0) {
        omFree(ip_block->m[0].data);
        ip_block->m[0].data = omStrDup("ip");
    }

    // Call rCompose with bitmask = 0 and check_comp = FALSE to avoid L(32767) and C
    ring result = rCompose(L, FALSE, 0, 0);
    if (!result) {
        WerrorS("Failed to compose new ring");
        omFreeBin(L, slists_bin);
        return R;
    }

   // std::cout << "[DEBUG] Result ring: " << rString(result) << std::endl;
    //std::cout << "[DEBUG] Result ring coefficient field type: " << nCoeffName(result->cf) << std::endl;

    omFreeBin(L, slists_bin);
    return result;
}

// Removes a parameter from a ring by index
ring removeParameter(ring R, int j) {
   // std::cout << "[DEBUG] ===== removeParameter called for index j = " << j << " =====" << std::endl;
   // std::cout << "[DEBUG] newR before removal: " << rString(R) << std::endl;

    if (j < 1 || j > rPar(R)) {
        Werror("Index out of range in removeParameter: j = %d, npars = %d", j, rPar(R));
        return R;
    }

    lists L = rDecompose(R);
    if (!L) {
        WerrorS("Failed to decompose ring");
        return R;
    }

    // Get the current parameters
    const char** params = rParameter(R);
    int npars = rPar(R);

    // Create a new parameter list excluding the j-th parameter
    char** new_params = (char**)omAlloc0((npars - 1) * sizeof(char*));
    int k = 0;
    for (int i = 0; i < npars; ++i) {
        if (i + 1 != j) {
            new_params[k++] = omStrDup(params[i]);
        }
    }

    // Update the coefficient field in the decomposed list
    if (L->m[0].rtyp == LIST_CMD) {
        lists cf_list = (lists)L->m[0].data;
        lists param_list = (lists)cf_list->m[1].data;

        // Clear old parameter list
        for (int i = 0; i <= param_list->nr; ++i) {
            omFree(param_list->m[i].data);
        }
        param_list->nr = npars - 2; // Adjust size (nr is 0-based)
        for (int i = 0; i < npars - 1; ++i) {
            param_list->m[i].rtyp = STRING_CMD;
            param_list->m[i].data = new_params[i]; // Transfer ownership
        }
    } else {
        WerrorS("Parameter removal only supported for rings with parameter lists");
        for (int i = 0; i < npars - 1; ++i) omFree(new_params[i]);
        omFree(new_params);
        omFreeBin(L, slists_bin);
        return R;
    }

    // Ensure ordering consistency (optional, but added for robustness)
    lists ordList = (lists)L->m[2].data;
    if (ordList->nr > 0) {
        for (int i = 1; i <= ordList->nr; ++i) {
            lists block = (lists)ordList->m[i].data;
            if (block) {
                omFree(block->m[0].data);
                delete (intvec*)block->m[1].data;
                omFreeBin(block, slists_bin);
            }
        }
        ordList->nr = 0; // Keep only the first ip block
    }

   // std::cout << "[DEBUG] ordList before rCompose: ";
    //printListAsString("ordList", ordList, true);

    // Use explicit rCompose call to avoid C and L(32767)
    ring result = rCompose(L, FALSE, 0, 0);
    if (!result) {
        WerrorS("Failed to compose new ring");
        for (int i = 0; i < npars - 1; ++i) omFree(new_params[i]);
        omFree(new_params);
        omFreeBin(L, slists_bin);
        return R;
    }

    //std::cout << "[DEBUG] newR after removal: " << rString(result) << std::endl;
    omFree(new_params); // Freed after transfer to param_list
    omFreeBin(L, slists_bin);
    return result;
}
// Assuming relevant headers and Singular are included

#include <set>
#include <algorithm>

LabeledGraph removeElimVars(const LabeledGraph& G) {
    std::cout << "[DEBUG] Starting removeElimVars" << std::endl;

    if (!G.elimvars || G.elimvars->nr < 0) {
        std::cout << "[DEBUG] No elimination variables to remove" << std::endl;
        return G;
    }

    LabeledGraph G1 = G;
    ring savedRing = currRing;
    ring R = G.over;
    ring RP = G.overpoly;
   // std::cout << "[DEBUG] R = " << rString(R) << std::endl;
   // std::cout << "[DEBUG] RP = " << rString(RP) << std::endl;

    lists el1 = G.elimvars;
    lists el = (lists)omAlloc0(sizeof(slists));
    int count = 0;
    for (int i = 0; i <= el1->nr; ++i)
        if (el1->m[i].rtyp == POLY_CMD && el1->m[i].data) count++;

    el->Init(count);
    el->nr = count - 1;
   // std::cout << "[DEBUG] Cleaned elimvars: ";
    int idx = 0;
    for (int i = 0; i <= el1->nr; ++i) {
        if (el1->m[i].rtyp == POLY_CMD && el1->m[i].data) {
            poly p = (poly)el1->m[i].data;
            el->m[idx].rtyp = POLY_CMD;
            el->m[idx].data = p_Copy(p, R);
            char* p_str = p_String((poly)el->m[idx].data, R);
           // std::cout << p_str << ", ";
            omFree(p_str);
            idx++;
        }
    }
   // std::cout << std::endl;

    std::vector<int> iv, ip;
    rChangeCurrRing(R);
    for (int i = 0; i <= el->nr; ++i) {
        if (el->m[i].rtyp != POLY_CMD || !el->m[i].data) continue;
        poly p = (poly)el->m[i].data;
        char* p_str = p_String(p, R);
        std::string p_str_clean = p_str;
        if (p_str_clean.front() == '(' && p_str_clean.back() == ')') {
            p_str_clean = p_str_clean.substr(1, p_str_clean.size() - 2);
        }
        int var_idx = p_Var(p, R);
        if (var_idx > 0 && var_idx <= rVar(R)) {
            iv.push_back(var_idx);
        } else {
            for (int j = 0; j < rPar(R); ++j) {
                if (strcmp(rParameter(R)[j], p_str_clean.c_str()) == 0) {
                    ip.push_back(j + 1);
                    break;
                }
            }
        }
        omFree(p_str);
    }

    std::sort(iv.begin(), iv.end(), std::greater<int>());
    std::sort(ip.begin(), ip.end(), std::greater<int>());

    ring R1 = R;
    for (int i : iv) {
        ring tempR = removeVariable(R1, i);
        if (!tempR) return G;
        if (R1 != R) rKill(R1);
        R1 = tempR;
       // std::cout << "[DEBUG] Result ring: " << rString(R1) << std::endl;
    }
    for (int i : ip) {
        ring tempR = removeParameter(R1, i);
        if (!tempR) return G;
        if (R1 != R) rKill(R1);
        R1 = tempR;
    }

   // std::cout << "[DEBUG] R1 after variable and parameter removal: " << rString(R1) << std::endl;

    // Map elimvars to RP
    rChangeCurrRing(RP);
    int nvars = rVar(R);
    int npars = rPar(R);
    int* perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
    int* par_perm = (int*)omAlloc0((npars + 1) * sizeof(int));
    for (int i = 1; i <= nvars; i++) perm[i] = i + npars;
    for (int i = 0; i < npars; i++) par_perm[i] = i + 1;
   // std::cout << "[DEBUG] par_perm: ";
   // for (int i = 0; i < npars; i++) std::cout << par_perm[i] << ", ";
   // std::cout << std::endl;
   // std::cout << "[DEBUG] perm: ";
   // for (int i = 0; i <= nvars; i++) std::cout << perm[i] << ", ";
   // std::cout << std::endl;

    nMapFunc nMap = n_SetMap(R->cf, RP->cf);

    for (int i = 0; i <= el->nr; ++i) {
        if (el->m[i].rtyp == POLY_CMD && el->m[i].data) {
            poly p = (poly)el->m[i].data;
            poly mapped = p_PermPoly(p, perm, R, RP, nMap, par_perm, npars, FALSE);
            char* str = p_String(mapped, RP);
            //std::cout << "[DEBUG] Mapped polynomial: " << str << std::endl;
            omFree(str);
            p_Delete(&mapped, RP);
        }
    }

    omFree(perm);
    omFree(par_perm);

    // Remove variables in RP
    std::vector<int> var_indices_RP;
    for (int i : iv) var_indices_RP.push_back(i + npars);
    for (int i : ip) var_indices_RP.push_back(i);
    std::sort(var_indices_RP.begin(), var_indices_RP.end(), std::greater<int>());
    ring RP1 = RP;
    for (int i : var_indices_RP) {
        ring tempRP = removeVariable(RP1, i);
        if (!tempRP) return G;
        if (RP1 != RP) rKill(RP1);
        RP1 = tempRP;
       // std::cout << "[DEBUG] Result ring: " << rString(RP1) << std::endl;
    }

   // std::cout << "[DEBUG] RP1 after overpoly variable removal: " << rString(RP1) << std::endl;

    // Map labels from R to R1
    rChangeCurrRing(R1);
    lists tr = (lists)omAlloc0(sizeof(slists));
    tr->Init(G.labels->nr + 1);
    tr->nr = G.labels->nr;
    nMap = n_SetMap(R->cf, R1->cf);

    for (int i = 0; i <= G.labels->nr; ++i) {
        if (G.labels->m[i].rtyp == POLY_CMD && G.labels->m[i].data) {
            poly p = (poly)G.labels->m[i].data;
            poly p_mapped = p_PermPoly(p, nullptr, R, R1, nMap, nullptr, 0, FALSE);
            tr->m[i].rtyp = POLY_CMD;
            tr->m[i].data = p_mapped;
        }
    }
   // std::cout << "[DEBUG] tr after mapping: ";
   // for (int i = 0; i <= tr->nr; ++i) {
   //     std::cout << pString((poly)tr->m[i].data) << ", ";
   // }
   // std::cout << std::endl;

    G1.over = R1;
    G1.overpoly = RP1;
    G1.labels = tr;
    G1.elimvars = (lists)omAlloc0(sizeof(slists));
    G1.elimvars->Init(0);
    G1.elimvars->nr = -1;

    rChangeCurrRing(savedRing);
    return G1;
}





LabeledGraph labelGraph(Graph G, int ch) {
    // Count unbounded edges (ct) and bounded edges (anzq)
    int ct = 0;
    int anzq = 0;
    //std::cout << "[DEBUG] Analyzing edges:" << std::endl;
    for (int i = 0; i <= G.edges->nr; i++) {
        lists edge = (lists)G.edges->m[i].data;
        if (edge->nr == 0) { // Unbounded edge (one vertex)
            ct++;
            int vertex = (int)(long)edge->m[0].data;
            //std::cout << "[DEBUG] Found unbounded edge [" << vertex << "] at index " << i << std::endl;
        } else if (edge->nr == 1) { // Bounded edge (two vertices)
            anzq++;
            int v1 = (int)(long)edge->m[0].data;
            int v2 = (int)(long)edge->m[1].data;
            //std::cout << "[DEBUG] Found bounded edge [" << v1 << "," << v2 << "] at index " << i << std::endl;
        }
    }

    //std::cout << "[DEBUG] ct (unbounded edges) = " << ct << "\n";
    //std::cout << "[DEBUG] anzq (bounded edges) = " << anzq << "\n";

    // Create base coefficient field (rationals QQ)
    coeffs baseCoeff = nInitChar(n_Q, NULL);
    if (!baseCoeff) {
        std::cerr << "Failed to create base coefficient field QQ.\n";
        return LabeledGraph();
    }
   // std::cout << "[DEBUG] Base coefficient field created (QQ).\n";

    // Create polynomial ring P: QQ[p(1..ct),q(1..anzq)], ip ordering
    std::vector<char*> varsP;
    //std::cout << "[DEBUG] Creating variables for ring Rp:" << std::endl;
    for (int i = 1; i <= ct; i++) {
        std::string name = "p(" + std::to_string(i) + ")";
        varsP.push_back(omStrDup(name.c_str()));
       // std::cout << "[DEBUG] Added p-variable: " << name << std::endl;
    }
    for (int i = 1; i <= anzq; i++) {
        std::string name = "q(" + std::to_string(i) + ")";
        varsP.push_back(omStrDup(name.c_str()));
        //std::cout << "[DEBUG] Added q-variable: " << name << std::endl;
    }

    ring P = rDefault(baseCoeff, varsP.size(), varsP.data(), ringorder_ip);
    rComplete(P);
    rChangeCurrRing(P);
    //std::cout << "[DEBUG] Ring R (overpoly) created: " << rString(P) << "\n";

    // Create extension coefficient field QQ(p(1),...,p(ct))
    std::vector<char*> varsCoeff;
    //std::cout << "[DEBUG] Creating coefficient field variables:" << std::endl;
    for (int i = 1; i <= ct; i++) {
        std::string name = "p(" + std::to_string(i) + ")";
        varsCoeff.push_back(omStrDup(name.c_str()));
        //std::cout << "[DEBUG] Added coefficient variable: " << name << std::endl;
    }

    ring coeffRing = rDefault(baseCoeff, ct, varsCoeff.data(), ringorder_dp);
    rComplete(coeffRing);
    TransExtInfo param = {coeffRing};
    coeffs extCoeff = nInitChar(n_transExt, &param);
    if (!extCoeff) {
        std::cerr << "Failed to create extension coefficient field.\n";
        return LabeledGraph();
    }
   // std::cout << "[DEBUG] Extension coefficient field created: QQ(p(1)..p(ct)).\n";

    // Create polynomial ring R: QQ(p(1),...,p(ct))[q(1..anzq)], ip ordering
    std::vector<char*> varsR;
    //std::cout << "[DEBUG] Creating variables for ring R:" << std::endl;
    for (int i = 1; i <= anzq; i++) {
        std::string name = "q(" + std::to_string(i) + ")";
        varsR.push_back(omStrDup(name.c_str()));
        //std::cout << "[DEBUG] Added variable: " << name << std::endl;
    }

    ring R = rDefault(extCoeff, anzq, varsR.data(), ringorder_ip);
    rComplete(R);
    //std::cout << "[DEBUG] Ring R (over) created: " << rString(R) << "\n";

    // Create labels in P explicitly
    ideal labelsP = idInit(G.edges->nr + 1, 1);
   int pidx = 1, qidx = 1;
for (int i = 0; i <= G.edges->nr; i++) {
    lists edge = (lists)G.edges->m[i].data;
    poly label = p_ISet(1, P);
    
    if (edge->nr == 0) {
        // unbounded edges: assign p(i)
        p_SetExp(label, pidx++, 1, P);  // p(i) variable
    } else {
        // bounded edges: assign q(i)
        p_SetExp(label, ct + qidx++, 1, P);  // q(i) variable (offset by number of p vars)
    }
    
    p_Setm(label, P);
    labelsP->m[i] = label;
}

    // Map labels from P to R
    rChangeCurrRing(R);
    lists labelsList = (lists)omAllocBin(slists_bin);
    labelsList->Init(G.edges->nr + 1);
    pidx = 1; qidx = 1;  // Reset indices
    for (int i = 0; i <= G.edges->nr; i++) {
        labelsList->m[i].rtyp = POLY_CMD;
        lists edge = (lists)G.edges->m[i].data;
        if (edge->nr == 0) {
            // For unbounded edges, use p(i) variables
            number n = n_Param(pidx++, extCoeff);
            poly p = p_One(R);
            p = p_Mult_nn(p, n, R);
            labelsList->m[i].data = p;
        } else {
            // For bounded edges, use q(i) variables
            poly p = p_One(R);
            p_SetExp(p, qidx++, 1, R);
            p_Setm(p, R);
            labelsList->m[i].data = p;
        }
    }

    LabeledGraph lG = makeLabeledGraph(G.vertices, G.edges, R, labelsList, P);

    // Cleanup
    for (char* v : varsP) omFree(v);
    for (char* v : varsCoeff) omFree(v);
    for (char* v : varsR) omFree(v);
    id_Delete(&labelsP, P);

    return lG;
}






ideal balancingIdeal(const LabeledGraph& G) {
    std::cout << "[DEBUG] Entering balancingIdeal\n";

    lists vertices = G.vertices;
    lists edges = G.edges;
    lists labels = G.labels;

    int numVertices = vertices->nr + 1;
    int numEdges = edges->nr + 1;
    //std::cout << "[DEBUG] numVertices: " << numVertices << std::endl;
    //std::cout << "[DEBUG] numEdges: " << numEdges << std::endl;
    //std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    ring R = G.over;
    //std::cout << "[DEBUG] Changing to ring G.over: " << rString(R) << std::endl;
    rChangeCurrRing(R);

    ideal I = idInit(numVertices + 1, 1);
    //std::cout << "[DEBUG] Initialized ideal with " << numVertices + 1 << " elements.\n";

    // Loop through each vertex
    for (int i = 0; i < numVertices; i++) {
        //std::cout << "[DEBUG] Processing vertex " << i+1 << std::endl;
        poly edg = NULL;

        // Loop through each edge
        for (int j = 0; j < numEdges; j++) {
            lists edge = (lists)edges->m[j].data;
            poly lab_j = (poly)labels->m[j].data;

            if (edge->nr == 0) {  // Unbounded edge (one vertex)
                int vertexInEdge = (int)(long)edge->m[0].data;
                if (vertexInEdge == (int)(long)vertices->m[i].data) {
                    //std::cout << "[DEBUG] Vertex " << vertexInEdge << " has unbounded edge with label: " << pString(lab_j) << std::endl;
                    edg = p_Add_q(edg, p_Copy(lab_j, R), R);
                }
            } else if (edge->nr == 1) {  // Bounded edge (two vertices)
                int vertex1 = (int)(long)edge->m[0].data;
                int vertex2 = (int)(long)edge->m[1].data;
                if (vertex1 == (int)(long)vertices->m[i].data) {
                    //std::cout << "[DEBUG] Vertex " << vertex1 << " is start of bounded edge (" << vertex1 << "," << vertex2 << ") with label: " << pString(lab_j) << std::endl;
                    edg = p_Add_q(edg, p_Copy(lab_j, R), R);
                }
                if (vertex2 == (int)(long)vertices->m[i].data) {
                    //std::cout << "[DEBUG] Vertex " << vertex2 << " is end of bounded edge (" << vertex1 << "," << vertex2 << ") with label: " << pString(lab_j) << std::endl;
                    edg = p_Sub(edg, p_Copy(lab_j, R), R);
                }
            }
        }

        if (edg == NULL) edg = p_ISet(0, R);
        //char* edgStr = pString(edg);
        //std::cout << "[DEBUG] Ideal element I[" << i << "] = " << edgStr << "\n";
        //omFree(edgStr);
        I->m[i] = edg;
    }

    // Calculate relation polynomial (sum of all unbounded edge labels)
    poly rel = NULL;
    //std::cout << "[DEBUG] Computing relation polynomial (sum of unbounded edges)\n";
    for (int j = 0; j < numEdges; j++) {
        lists edge = (lists)edges->m[j].data;
        if (edge->nr == 0) { // Unbounded edge
            poly lab_j = (poly)labels->m[j].data;
            //std::cout << "[DEBUG] Adding unbounded edge label: " << pString(lab_j) << " to relation polynomial\n";
            rel = p_Add_q(rel, p_Copy(lab_j, R), R);
        }
    }
    if (rel == NULL) rel = p_ISet(0, R);
    //char* relStr = pString(rel);
    //std::cout << "[DEBUG] Relation polynomial (final element) I[" << numVertices << "] = " << relStr << "\n";
    //omFree(relStr);
    I->m[numVertices] = rel;

    std::cout << "[DEBUG] Exiting balancingIdeal\n";
    return I;
}

// Substitute one polynomial for another using p_SubstPoly
poly substitutePoly(poly label, poly a, poly b, ring r) {
    if (!label || !a || !b) return NULL;
    
    // Create coefficient mapping function
    nMapFunc nMap = [](number n, const coeffs cf1, const coeffs cf2) -> number {
        if (n == NULL) return NULL;
        if (n_IsZero(n, cf1)) return n_Init(0, cf2);
        if (n_IsOne(n, cf1)) return n_Init(1, cf2);
        if (n_IsMOne(n, cf1)) return n_Init(-1, cf2);
        return n_Copy(n, cf1);
    };

    // Get variable index from polynomial a
    int var = 0;
    for (int j = 1; j <= rVar(r); j++) {
        if (p_GetExp(a, j, r) > 0) {
            var = j;
            break;
        }
    }

    // If no variable found in a, we can't do substitution
    if (var == 0) return p_Copy(label, r);

    // Do the substitution using Singular's p_SubstPoly
    poly result = p_SubstPoly(p_Copy(label, r), var, p_Copy(b, r), r, r, nMap);
    if (result) p_Normalize(result, r);
    return result;
}

LabeledGraph substituteGraph(LabeledGraph G, poly a, poly b) {
    ring savedRing = currRing;
    rChangeCurrRing(G.over);

    // Create a new list for the substituted labels
    lists L = (lists)omAlloc0(sizeof(slists));
    L->Init(G.labels->nr + 1);
    L->nr = G.labels->nr;
    
    // Process each label
    for (int i = 0; i <= G.labels->nr; i++) {
        L->m[i].Init();
        L->m[i].rtyp = POLY_CMD;
        poly label = (poly)G.labels->m[i].Data();
        if (!label) {
            L->m[i].data = NULL;
            continue;
        }

        poly result;
        if (i == G.labels->nr && p_ComparePolys(label, a, G.over)) {
            result = p_Copy(b, G.over);
        } else {
            result = substitutePoly(label, a, b, G.over);
        }
        L->m[i].data = result;
    }

    // Create a new labeled graph with the substituted labels
    LabeledGraph G1 = makeLabeledGraph(G.vertices, G.edges, G.over, L, G.overpoly);
    rChangeCurrRing(savedRing);
    return G1;
}

LabeledGraph eliminateVariables(LabeledGraph G) {
    std::cout << "[DEBUG] Entering eliminateVariables" << std::endl;
    ring savedRing = currRing;

    std::cout << "[DEBUG] G.over ring: " << rString(G.over) << std::endl;
    std::cout << "[DEBUG] G.overpoly ring: " << rString(G.overpoly) << std::endl;
    std::cout << "[DEBUG] coefficients: " << rPar(G.over) << std::endl;
    rChangeCurrRing(G.over);
    ideal I = balancingIdeal(G);
    std::cout << "[DEBUG] Ideal computed" << std::endl;
    for (int i = 0; i < IDELEMS(I); i++) {
        if (!I->m[i]) continue;
        char* s = p_String(I->m[i], G.over);
        omFree(s);
    }

    std::cout << "[DEBUG] Mapping ideal from G.over to G.overpoly" << std::endl;
    int nvars = rVar(G.over);
    int npars = rPar(G.over);
    int *perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
    int *par_perm = (int*)omAlloc0((npars + 1) * sizeof(int));

    for (int i = 1; i <= nvars; i++) perm[i] = i + npars;
    for (int i = 0; i < npars; i++) par_perm[i] = i + 1;

    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);
    ideal I_mapped = id_PermIdeal(I, 1, IDELEMS(I), perm, G.over, G.overpoly, nMap, par_perm, npars, FALSE);

    rChangeCurrRing(G.overpoly);
    si_opt_1 |= Sy_bit(OPT_REDTAIL);
    ideal I_std = kStd(I_mapped, NULL, testHomog, NULL, NULL, 0, TRUE);

    LabeledGraph G1 = G;
    lists eliminatedVars = (lists)omAlloc0(sizeof(slists));
    eliminatedVars->Init(IDELEMS(I_std));
    eliminatedVars->nr = IDELEMS(I_std) - 1;


for (int i = 0; i < IDELEMS(I_std); i++) {
    eliminatedVars->m[i].Init();
    eliminatedVars->m[i].rtyp = POLY_CMD;
    eliminatedVars->m[i].data = NULL;

    poly f = I_std->m[i];
    if (!f) continue;

    if (p_IsConstant(f, G.overpoly)) {
        continue;
    }

    poly ld = p_Head(f, G.overpoly);
    poly ta = p_Sub(p_Copy(ld, G.overpoly), p_Copy(f, G.overpoly), G.overpoly);
    
    // Map polynomials between rings
    // Create permutation for variables
    int nvars = rVar(G.overpoly);
    int* perm = (int*)omAlloc0((nvars+1)*sizeof(int));
    
    int npars1 = rPar(G.over);
    int nvars1 = rVar(G.over);
    
    // Initialize all entries to 0
    for(int j = 0; j <= nvars; j++) {
        perm[j] = 0;
    }
    
    // Map coefficients p(i) to negative indices
    for(int j = 1; j <= npars1; j++) {
        perm[j] = -j;
    }
    
    // Map variables q(i) to positive indices
    for(int j = npars1+1; j <= nvars; j++) {
        perm[j] = j-npars1;
    }
    
    // Create coefficient mapping function
    nMapFunc nMap = [](number n, const coeffs cf1, const coeffs cf2) -> number {
        if (n == NULL) return NULL;
        if (n_IsZero(n, cf1)) return n_Init(0, cf2);
        if (n_IsOne(n, cf1)) return n_Init(1, cf2);
        if (n_IsMOne(n, cf1)) return n_Init(-1, cf2);
        return n_Copy(n, cf1);
    };
    
    // Map leading term
    poly ld_in_R = NULL;
    if (ld) {
        ld_in_R = p_PermPoly(ld, perm, G.overpoly, G.over, nMap, NULL, 0, FALSE);
    }
    
    // Map tail term
    poly ta_in_R = NULL;
    if (ta) {
        ta_in_R = p_PermPoly(ta, perm, G.overpoly, G.over, nMap, NULL, 0, FALSE);
    }
    
    // Free memory
    omFree(perm);


    eliminatedVars->m[i].data = p_Copy(ld_in_R, G.over);
    G1 = substituteGraph(G1, ld_in_R, ta_in_R);
    // Clean up
    p_Delete(&ld, G.overpoly);
    p_Delete(&ta, G.overpoly);
    p_Delete(&ld_in_R, G.over);
    p_Delete(&ta_in_R, G.over);
    
    rChangeCurrRing(G.overpoly);
}
    G1.elimvars = eliminatedVars;
    std::cout << "[DEBUG] Elimination complete. Result:" << std::endl;
    printLabeledGraph(G1);

    id_Delete(&I, G.over);
    id_Delete(&I_mapped, G.overpoly);
    id_Delete(&I_std, G.overpoly);

    rChangeCurrRing(savedRing);
    return G1;
}
// Helper: Create a polynomial list
lists createPolyList(const char* vars[], int size, ring r) {
    lists L = (lists)omAlloc(sizeof(sleftv));
    L->Init(size);
    for (int i = 0; i < size; i++) {
        poly p = p_ISet(1, r);
        // Find variable index by comparing names
        int varIndex = -1;
        for (int j = 1; j <= rVar(r); j++) {
            if (strcmp(vars[i], rRingVar(j-1, r)) == 0) {
                varIndex = j;
                break;
            }
        }
        if (varIndex != -1) {
            p_SetExp(p, varIndex, 1, r);
        }
        p_Setm(p, r);
        L->m[i].rtyp = POLY_CMD;
        L->m[i].data = (void*)p;
    }
    L->nr = size - 1;
    return L;
}

ideal propagators(const LabeledGraph& G) {
    ring savedRing = currRing;
    rChangeCurrRing(G.over);

    // Step 1: Create ideal J for internal edges
    ideal J = idInit(G.labels->nr);
    int j_idx = 0;
    for (int i = 0; i <= G.labels->nr; ++i) {
        if (G.edges->m[i].Typ() == LIST_CMD && ((lists)G.edges->m[i].Data())->nr + 1 == 2) {
            poly label = (poly)G.labels->m[i].Data();
            if (label) {
                J->m[j_idx++] = p_Mult_q(p_Copy(label, G.over), p_Copy(label, G.over), G.over);
            }
        }
    }
    J->ncols = j_idx;

    // Step 2: Create ideal infedges for external edges
    ideal infedges = idInit(G.labels->nr);
    int inf_idx = 0;
    for (int i = 0; i <= G.labels->nr; ++i) {
        if (G.edges->m[i].Typ() == LIST_CMD && ((lists)G.edges->m[i].Data())->nr + 1 == 1) {
            poly label = (poly)G.labels->m[i].Data();
            if (label) {
                infedges->m[inf_idx++] = p_Mult_q(p_Copy(label, G.over), p_Copy(label, G.over), G.over);
            }
        }
    }
    infedges->ncols = inf_idx;

    // Step 3: Prepare permutations for mapping to G.overpoly
    int nvars_over = rVar(G.over);
    int npars_over = rPar(G.over);
    int* perm = (int*)omAlloc0((nvars_over + 1) * sizeof(int));
    int* par_perm = (int*)omAlloc0((npars_over + 1) * sizeof(int));
    for (int i = 1; i <= nvars_over; i++) perm[i] = i + npars_over;
    for (int i = 0; i < npars_over; i++) par_perm[i] = i + 1;
    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);

    // Step 4: Map J and infedges to G.overpoly
    ideal Jpoly = id_PermIdeal(J, 1, IDELEMS(J), perm, G.over, G.overpoly, nMap, par_perm, npars_over, FALSE);
    ideal infpoly = id_PermIdeal(infedges, 1, IDELEMS(infedges), perm, G.over, G.overpoly, nMap, par_perm, npars_over, FALSE);

    // Step 5: Compute standard basis and reduce in G.overpoly
    rChangeCurrRing(G.overpoly);
    ideal std_inf = kStd(infpoly, NULL, testHomog, NULL, NULL, 0, 0);
    //std::cout << "[DEBUG] Checking rings before reduction:" << std::endl;
    //std::cout << "Jpoly ring: " << rString(G.overpoly) << std::endl;
    //std::cout << "std_inf ring: " << rString(G.overpoly) << std::endl;
    //std::cout << "G.overpoly: " << rString(G.overpoly) << std::endl;
    ideal Jred = kNF(std_inf, std_inf, Jpoly, 0, 0);
    // Step 6: Map back to G.over
    rChangeCurrRing(G.over);
    int nvars_overpoly = rVar(G.overpoly);
    int* perm_back = (int*)omAlloc0((nvars_overpoly + 1) * sizeof(int));
    for (int i = 1; i <= npars_over; i++) perm_back[i] = -i;
    for (int i = npars_over + 1; i <= nvars_overpoly; i++) perm_back[i] = i - npars_over;
    nMap = n_SetMap(G.overpoly->cf, G.over->cf);
    ideal result = id_PermIdeal(Jred, 1, IDELEMS(Jred), perm_back, G.overpoly, G.over, nMap, NULL, 0, FALSE);

    // Step 7: Clean up
    id_Delete(&J, G.over);
    id_Delete(&infedges, G.over);
    id_Delete(&Jpoly, G.overpoly);
    id_Delete(&infpoly, G.overpoly);
    id_Delete(&std_inf, G.overpoly);
    id_Delete(&Jred, G.overpoly);
    omFree(perm);
    omFree(par_perm);
    omFree(perm_back);

    rChangeCurrRing(savedRing);
    return result;
}
ideal ISP(const LabeledGraph& G) {
    std::cout << "[DEBUG] Entering ISP" << std::endl;
    ring savedRing = currRing;
    rChangeCurrRing(G.over);
   // std::cout<<"print vertices"<<std::endl;
    //printListAsString("G.vertices", G.vertices, true, 1);
    //std::cout<<"print edges"<<std::endl;
    //printListAsString("G.edges", G.edges, true, 1);

    //std::cout<<"print labels"<<std::endl;
    //printPolyListAsVector("G.labels", G.labels, G.over);
    
    //std::cout << "[DEBUG] Computing propagators J:" << std::endl;
    ideal J = propagators(G);
    //printIdeal(J);
    
    // External edges - modified to match Singular version
    //std::cout << "[DEBUG] Computing infedges:" << std::endl;
    int num_edges = G.edges ? G.edges->nr + 1 : 0;
    ideal infedges = idInit(num_edges + 1, 1);  // Initialize with correct size
    for (int i = 0; i < num_edges; ++i) {
        if (G.edges->m[i].Typ() == LIST_CMD && 
            ((lists)G.edges->m[i].Data())->nr + 1 == 1) {
            poly label = (poly)G.labels->m[i].Data();
            if (label) {
                infedges->m[i] = p_Copy(label, G.over);
            }
        }
    }
    //std::cout << "[DEBUG] infedges:" << std::endl;
    //printIdeal(infedges);

    // Map to overpoly ring
    //std::cout << "[DEBUG] Mapping to overpoly ring:" << std::endl;
    rChangeCurrRing(G.overpoly);
    int nvars_over = rVar(G.over);
    int npars_over = rPar(G.over);
    int* perm = (int*)omAlloc0((nvars_over + 1) * sizeof(int));
    int* par_perm = (int*)omAlloc0((npars_over + 1) * sizeof(int));
    for (int i = 1; i <= nvars_over; i++) perm[i] = i + npars_over;
    for (int i = 0; i < npars_over; i++) par_perm[i] = i + 1;
    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);
    
    //std::cout << "[DEBUG] Mapping J to overpoly:" << std::endl;
    ideal J_mapped = id_PermIdeal(J, 1, IDELEMS(J), perm, G.over, G.overpoly, nMap, par_perm, npars_over, FALSE);
    //printIdeal(J_mapped);
    
    //std::cout << "[DEBUG] Mapping infedges to overpoly:" << std::endl;
    ideal infedges_mapped = id_PermIdeal(infedges, 1, IDELEMS(infedges), perm, G.over, G.overpoly, nMap, par_perm, npars_over, FALSE);
   // printIdeal(infedges_mapped);
    
    // Add infedges^2 to J
    ideal J_with_inf = id_Add(J_mapped, id_Mult(infedges_mapped, infedges_mapped, G.overpoly), G.overpoly);
   // printIdeal(J_with_inf);
    


 // Add elimvars if they exist
if (G.elimvars && G.elimvars->nr >= 0) {
    std::cout << "[DEBUG] Adding elimvars in overpoly:" << std::endl;

    for (int i = 0; i <= G.elimvars->nr; ++i) {
        if (G.elimvars->m[i].Typ() != POLY_CMD) continue;

        poly p = (poly)G.elimvars->m[i].Data();
        if (!p) continue;

        poly p_mapped = p_PermPoly(p, perm, G.over, G.overpoly, nMap, par_perm, npars_over, FALSE);
        if (!p_mapped) continue;

        ideal temp = idInit(1, 1);
        temp->m[0] = p_mapped;
        J_with_inf = id_Add(J_with_inf, temp, G.overpoly);
        id_Delete(&temp, G.overpoly);
    }
}


    // Compute standard basis
//    std::cout << "[DEBUG] Computing standard basis:" << std::endl;
    ideal J_std = kStd(J_with_inf, NULL, isHomog, NULL, NULL, 0, 0, NULL, NULL);
   // printIdeal(J_std);
    
    // Get kbase
//    std::cout << "[DEBUG] Computing kbase:" << std::endl;
    ideal L = scKBase(2, J_std);
   // printIdeal(L);
    
    // Extract polynomials from kbase
//    std::cout << "[DEBUG] Extracting polynomials from kbase:" << std::endl;
    ideal I = idInit(IDELEMS(L), 1);
    for (int i = 0; i < IDELEMS(L); ++i) {
        if (L->m[i]) {
            I->m[i] = p_Copy(L->m[i], G.overpoly);
        }
    }
   // printIdeal(I);
    
    // Map back to original ring
//    std::cout << "[DEBUG] Mapping back to original ring:" << std::endl;
    rChangeCurrRing(G.over);
    
    
    
    // Create permutation arrays for mapping back
    int nvars_overpoly = rVar(G.overpoly);
    int* perm_back = (int*)omAlloc0((nvars_overpoly + 1) * sizeof(int));
    for (int i = 1; i <= npars_over; i++) perm_back[i] = -i;
    for (int i = npars_over + 1; i <= nvars_overpoly; i++) perm_back[i] = i - npars_over;

    // Update nMap for mapping back
    nMap = n_SetMap(G.overpoly->cf, G.over->cf);

    ideal K = id_PermIdeal(I, 1, IDELEMS(I), perm_back, G.overpoly, G.over, nMap, NULL, 0, FALSE);
   // printIdeal(K);
    
    // Clean up
    id_Delete(&J, G.over);
    id_Delete(&infedges, G.over);
    id_Delete(&J_mapped, G.overpoly);
    id_Delete(&infedges_mapped, G.overpoly);
    id_Delete(&J_with_inf, G.overpoly);
    id_Delete(&J_std, G.overpoly);
    id_Delete(&L, G.overpoly);
    id_Delete(&I, G.overpoly);
    omFree(perm);
    omFree(par_perm);
    omFree(perm_back);

    rChangeCurrRing(savedRing);
    return K;
}



matrix buildZVars(const ring Z, int n, int m2, const matrix& pq) {
    matrix zvars = mpNew(1, n + m2);

    // z(1)..z(n)
    for (int i = 0; i < n; ++i) {
        poly z = p_One(Z);
        p_SetExp(z, i + 1 + m2, 1, Z);  // skip t vars
        p_Setm(z, Z);
        zvars->m[i] = z;
    }

    // pq[1..m2] (assumed to be 1x m2 matrix)
    for (int i = 0; i < m2; ++i) {
        zvars->m[n + i] = p_Copy(pq->m[i], Z);
    }

    return zvars;
}
LabeledGraph computeBaikovMatrix(const Graph& G0) {
    std::cout << "[DEBUG] Entering computeBaikovMatrix for Graph type" << std::endl;
    
    if (!G0.vertices || !G0.edges) {
        std::cerr << "[ERROR] Invalid input graph" << std::endl;
        return LabeledGraph();
    }

    std::cout << "[DEBUG] Converting Graph to LabeledGraph" << std::endl;
    LabeledGraph lG = labelGraph(G0, 0);
    
    std::cout << "[DEBUG] Eliminating variables" << std::endl;
    LabeledGraph G1 = eliminateVariables(lG);
    std::cout << "[DEBUG] After eliminateVariables: ring RP = G1.overpoly = " << rString(G1.overpoly) << std::endl;
    std::cout << "[DEBUG] After eliminateVariables: ring R = G1.over = " << rString(G1.over) << std::endl;
    
    std::cout << "[DEBUG] Removing eliminated variables" << std::endl;
    LabeledGraph G2 = removeElimVars(G1);
    std::cout << "[DEBUG] After removeElimVars: ring RP = G2.overpoly = " << rString(G2.overpoly) << std::endl;
    std::cout << "[DEBUG] After removeElimVars: ring R = G2.over = " << rString(G2.over) << std::endl;
    
    ring savedRing = currRing;
    LabeledGraph result = computeBaikovMatrix(G2);
    rChangeCurrRing(savedRing);
    return result;
}



matrix id_Module2Matrix(ideal mod, const ring R)
{
    std::cout << "[DEBUG] Entering id_Module2Matrix" << std::endl;
  matrix result = mpNew(mod->rank,IDELEMS(mod));
  std::cout << "[DEBUG] id_Module2Matrix rank = " << mod->rank << std::endl;
  //std::cout<<" matrix result is " << matrixToString(result, R) << std::endl;
  long i; long cp;
  poly p,h;

  for(i=0;i<IDELEMS(mod);i++)
  {
    p=pReverse(mod->m[i]);
    mod->m[i]=NULL;
    while (p!=NULL)
    {
      h=p;
      pIter(p);
      pNext(h)=NULL;
      cp = si_max(1L,p_GetComp(h, R));     // if used for ideals too
      //cp = p_GetComp(h,R);
      p_SetComp(h,0,R);
      p_SetmComp(h,R);
//#ifdef TEST
      if (cp>mod->rank)
      {
        Print("## inv. rank %ld -> %ld\n",mod->rank,cp);
        int k,l,o=mod->rank;
        mod->rank=cp;
        matrix d=mpNew(mod->rank,IDELEMS(mod));
        for (l=0; l<o; l++)
        {
          for (k=0; k<IDELEMS(mod); k++)
          {
            MATELEM0(d,l,k)=MATELEM0(result,l,k);
            MATELEM0(result,l,k)=NULL;
          }
        }
        id_Delete((ideal *)&result,R);
        result=d;
      }
//#endif
      MATELEM0(result,cp-1,i) = p_Add_q(MATELEM0(result,cp-1,i),h,R);
    }
  }
  //std::cout<<" matrix result in   id_Module2Matrix" << matrixToString(result, R) << std::endl;

  // obachman 10/99: added the following line, otherwise memory leak!
  id_Delete(&mod,R);
  return result;
}



matrix id_Module2formatedMatrix(ideal mod,int rows, int cols, const ring R)
{
    rows=1;
    std::cout<<"print rows, cols" << rows << "," << cols << std::endl;
  matrix result = mpNew(rows,cols);
  int i,cp,r=id_RankFreeModule(mod,R),c=IDELEMS(mod);
  poly p,h;
std::cout<<"i,cp,r,c=" << i << "," << cp << "," << r << "," << c << std::endl;
std::cout<<"ring R=" << rString(R) << std::endl;

  if (r>rows) r = rows;
  if (c>cols) c = cols;
  std::cout<<"print ideal mod" ;
for(int i=0;i<IDELEMS(mod);i++){
    std::cout<<" mod->m["<<i<<"]=" << pString((poly)mod->m[i]) << std::endl;
}
  for(i=0;i<c;i++)
  {
    p=pReverse(mod->m[i]);
    std::cout<<"poly p=" << p_String(p, R) <<" mod->m[i]=" << pString((poly)mod->m[i]) << std::endl;
    mod->m[i]=NULL;
    while (p!=NULL)
    {
      std::cout<<"!p=NULL is " << p_String(p, R) << std::endl;
      h=p;
      pIter(p);
      pNext(h)=NULL;
      std::cout<<"h=" << p_String(h, R) << std::endl;
      cp = p_GetComp(h,R);
      std::cout<<"cp=" << cp << std::endl;
      if (cp<=r)
      {
        std::cout<<"cp<=r" << std::endl;
        p_SetComp(h,0,R);
        p_SetmComp(h,R);
        std::cout<<"print h=" << p_String(h, R) << std::endl;
        std::cout<<" cp-1,i=" << cp-1 << "," << i << std::endl;
        std::cout<<"p_Add_q(MATELEM0(result,cp-1,i),h,R)"<<p_String(MATELEM0(result,cp-1,i), R) << std::endl;
        MATELEM0(result,cp-1,i) = p_Add_q(MATELEM0(result,cp-1,i),h,R);
        std::cout<<"MATELEM0(result,cp-1,i)=" << p_String(MATELEM0(result,cp-1,i), R) << std::endl;
      }
      else
        p_Delete(&h,R);
    }
  }
  id_Delete(&mod,R);
  return result;
}


ideal id_ResizeModule(ideal mod,int rows, int cols, const ring R)
{
  // columns?
  if (cols!=IDELEMS(mod))
  {
    for(int i=IDELEMS(mod)-1;i>=cols;i--) p_Delete(&mod->m[i],R);
    pEnlargeSet(&(mod->m),IDELEMS(mod),cols-IDELEMS(mod));
    IDELEMS(mod)=cols;
  }
  // rows?
  if (rows<mod->rank)
  {
    for(int i=IDELEMS(mod)-1;i>=0;i--)
    {
      if (mod->m[i]!=NULL)
      {
        while((mod->m[i]!=NULL) && (p_GetComp(mod->m[i],R)>rows))
          mod->m[i]=p_LmDeleteAndNext(mod->m[i],R);
        poly p=mod->m[i];
        while(pNext(p)!=NULL)
        {
          if (p_GetComp(pNext(p),R)>rows)
            pNext(p)=p_LmDeleteAndNext(pNext(p),R);
          else
            pIter(p);
        }
      }
    }
  }
  mod->rank=rows;
  return mod;
}

/*2
* substitute the n-th variable by the monomial e in id
* destroy id
*/
ideal  id_Subst(ideal id, int n, poly e, const ring r)
{
  int k=MATROWS((matrix)id)*MATCOLS((matrix)id);
  ideal res=(ideal)mpNew(MATROWS((matrix)id),MATCOLS((matrix)id));

  res->rank = id->rank;
  for(k--;k>=0;k--)
  {
    res->m[k]=p_Subst(id->m[k],n,e,r);
    id->m[k]=NULL;
  }
  id_Delete(&id,r);
  return res;
}
/* 
matrix matIdLift(ideal Gomega, ideal M)
{
  std::cout << "[DEBUG] Computing matIdLift..." << std::endl;

  // Compute the lift module
  ideal Mtmp = idLift(Gomega, M, NULL, FALSE, FALSE, TRUE, NULL);
  if (!Mtmp) {
    std::cerr << "[ERROR] idLift returned NULL" << std::endl;
    return NULL;
  }

  // Print module entries
  std::cout << "[DEBUG] Mtmp = ";
  for (int i = 0; i < IDELEMS(Mtmp); i++) {
    if (Mtmp->m[i]) {
      std::cout << p_String(Mtmp->m[i], currRing) << " ";
    } else {
      std::cout << "0 ";
    }
  }
  std::cout << std::endl;

  // Set proper matrix dimensions
  int rows = IDELEMS(Gomega);
  int cols = IDELEMS(Mtmp);
  Mtmp->rank = rows;

  // Resize safely to avoid issues with invalid components
  Mtmp = id_ResizeModule(Mtmp, rows, cols, currRing);

  // Convert module to matrix
  matrix res = id_Module2formatedMatrix(Mtmp, rows, cols, currRing);
  std::cout << "[DEBUG] Lift matrix = " << std::endl;
  printMatrix(res);

  return res;
}
 */


#include <sstream>

std::string matrixToString(matrix m, const ring R) {
    std::stringstream ss;
    for (int i = 0; i < MATROWS(m); ++i) {
        for (int j = 0; j < MATCOLS(m); ++j) {
            poly entry = MATELEM(m, i + 1, j + 1);
            if (entry) {
                char* s = p_String(entry, R);
                ss << s << ",";
                omFree(s);
            } else {
                ss << "0,";
            }
        }
        ss << "\n";
    }
    return ss.str();
}

LabeledGraph computeBaikovMatrix(const LabeledGraph& G0) {
    std::cout << "[DEBUG] computeBaikovMatrix started for LabeledGraph" << std::endl;
    
    std::cout << "[DEBUG] Deep copying labeled graph" << std::endl;
    LabeledGraph G = G0;
    
    ring savedRing = currRing;
    rChangeCurrRing(G.over);
    std::cout << "[DEBUG] Initial ring R = G.over = " << rString(G.over) << std::endl;
    std::cout << "[DEBUG] Initial ring RP = G.overpoly = " << rString(G.overpoly) << std::endl;
    
    std::cout << "[DEBUG] Computing propagators P" << std::endl;
    ideal P = propagators(G);
    printIdeal(P);
    
    std::cout << "[DEBUG] Computing ISP I" << std::endl;
    ideal I = ISP(G);
    printIdeal(I);
    
    std::cout << "[DEBUG] Combining P and I to PI" << std::endl;
    ideal PI = id_Add(P, I, G.over);
    printIdeal(PI);

    if (IDELEMS(PI) == 1 && p_IsConstant(PI->m[0], G.over) && n_IsOne(p_GetCoeff(PI->m[0], G.over), G.over->cf)) {
        std::cerr << "[ERROR] PI collapsed to <1>, check preprocessing" << std::endl;
        id_Delete(&P, G.over);
        id_Delete(&I, G.over);
        id_Delete(&PI, G.over);
        rChangeCurrRing(savedRing);
        return G;
    }
    id_Delete(&P, G.over);
    id_Delete(&I, G.over);

    rChangeCurrRing(G.overpoly);
    std::cout << "[DEBUG] Mapping PI to RP = G.overpoly ring" << std::endl;
    int nvars = rVar(G.over);
    int npars = rPar(G.over);
    int* perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
    int* par_perm = (int*)omAlloc0((npars + 1) * sizeof(int));
    for (int i = 1; i <= nvars; ++i) perm[i] = i + npars;
    for (int i = 0; i < npars; ++i) par_perm[i] = i + 1;
    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);
    ideal PI_mapped = id_PermIdeal(PI, 1, IDELEMS(PI), perm, G.over, G.overpoly, nMap, par_perm, npars, FALSE);
    std::cout << "[DEBUG] Mapped PI = " << std::endl;
    printIdeal(PI_mapped);
    id_Delete(&PI, G.over);

    std::cout << "[DEBUG] Building Gram matrix" << std::endl;
    int nvars1 = rVar(G.overpoly);
    int startvars = npars + 1;
    std::cout << "[DEBUG] startvars = " << startvars << std::endl;
    int gramSize = nvars1 * nvars1;
    ideal gram = idInit(gramSize, 1);

    int idx = 0;
    for (int i = 1; i <= nvars1; i++) {
        for (int j = 1; j <= nvars1; j++) {
            if (i >= startvars || j >= startvars) {
                poly p = p_One(G.overpoly);
                p_SetExp(p, i, 1, G.overpoly);
                p_Setm(p, G.overpoly);
                poly q = p_One(G.overpoly);
                p_SetExp(q, j, 1, G.overpoly);
                p_Setm(q, G.overpoly);
                poly prod = p_Mult_q(p, q, G.overpoly);
                gram->m[idx] = prod;
            } else {
                gram->m[idx] = NULL;
            }
            idx++;
        }
    }

    std::cout << "[DEBUG] gramSize = " << gramSize << ", idelems(gram) = " << IDELEMS(gram) << std::endl;
    std::cout << "[DEBUG] Gram ideal = ";
   // printIdeal(gram);
    for(int i=0; i<IDELEMS(gram); i++){
        std::cout<<"[DEBUG] [" << i << "]" << p_String(gram->m[i], G.overpoly) << std::endl;
    }
    std::cout << "[DEBUG] Adding scalar products to PI_mapped" << std::endl;
    for (int i = 1; i < npars; ++i) {
        for (int j = i + 1; j <= npars; ++j) {
            poly prod = p_One(G.overpoly);
            p_SetExp(prod, i, 1, G.overpoly);
            p_SetExp(prod, j, 1, G.overpoly);
            p_Setm(prod, G.overpoly);
            ideal temp = idInit(1, 1);
            temp->m[0] = prod;
            PI_mapped = id_Add(PI_mapped, temp, G.overpoly);
            id_Delete(&temp, G.overpoly);
        }
    }
    std::cout << "[DEBUG] size of PI_mapped = " << IDELEMS(PI_mapped) << ", PI with scalar products = ";
    printIdeal(PI_mapped);

std::cout<<"[DEBUG] PI_mapped" << std::endl;
matrix PI_mat=id_Module2Matrix(PI_mapped, currRing);
    std::cout << "[DEBUG] Computing PI_mat of s" << std::endl;
    printMatrix(PI_mat);
matrix gram_mat=id_Module2Matrix(gram, currRing);
    ideal PI_mod=id_Matrix2Module(PI_mat, currRing);
    printIdeal(PI_mod);
    ideal gram_mod=id_Matrix2Module(gram_mat, currRing);
    printIdeal(gram_mod);
    // Step 1: Compute A_ideal without U to get the correct lift cefficients
    std::cout << "[DEBUG] Computing lift matrix" << std::endl;
   
    ideal rest = NULL;
    matrix U=NULL;
// Try different algorithm if default fails
ideal A_ideal = idLift(PI_mod, gram_mod, NULL, FALSE, TRUE, TRUE, NULL, GbDefault);
   std::cout<<"print A_ideal" << std::endl;
   printIdeal(A_ideal);
    
  //  U=id_Module2Matrix(PI_mod, currRing);
    matrix AA=id_Module2Matrix(A_ideal,currRing); 
   std::cout<<" [DEBUG] Before U"<<std::endl;
   // matrix U=matIdLift(PI_mapped, gram); // Compute the lift matrix (A M)

    std::cout << "[DEBUG] Lift matrix U dimensions = " << U->nrows << "x" << U->ncols << std::endl;
    std::cout << "[DEBUG] Lift matrix U = " << std::endl;
    std::cout << matrixToString(U, currRing);
    // Construct Baikov ring and matrix B
    int m = npars;
    int m2 = (m * (m - 1)) / 2;
    int mt = m2 - 1;
    int n = IDELEMS(PI_mapped) - m2;

    std::cout << "[DEBUG] Constructing Baikov ring" << std::endl;
    char** extNames = (char**)omAlloc0((mt + 1) * sizeof(char*));
    for (int i = 0; i < mt; ++i) extNames[i] = omStrDup(("t(" + std::to_string(i + 1) + ")").c_str());
    extNames[mt] = omStrDup("D");
    char** varNames = (char**)omAlloc0(n * sizeof(char*));
    for (int i = 0; i < n; ++i) varNames[i] = omStrDup(("z(" + std::to_string(i + 1) + ")").c_str());
    ring Z = createRing(extNames, mt + 1, varNames, n, ringorder_dp);
    std::cout << "[DEBUG] Baikov ring Z = " << rString(Z) << std::endl;

    std::cout << "[DEBUG] Building symmetric matrix B" << std::endl;
    rChangeCurrRing(Z);
    matrix B = mpNew(nvars1, nvars1);
    matrix pq = mpNew(1, m2);
    poly sumt = NULL;
    idx = 0;
    for (int i = 1; i <= m; ++i) {
        for (int j = i + 1; j <= m; ++j) {
            poly entry = NULL;
            if (idx < mt) {
                entry = p_One(Z);
                p_SetExp(entry, idx + 1, 1, Z);
                p_Setm(entry, Z);
                entry = p_Mult_nn(entry, n_Div(n_Init(1, Z->cf), n_Init(2, Z->cf), Z->cf), Z);
                pq->m[idx] = p_Copy(entry, Z);
                sumt = p_Add_q(sumt, p_Copy(entry, Z), Z);
            } else {
                entry = p_Neg(p_Copy(sumt, Z), Z);
                pq->m[idx] = p_Copy(entry, Z);
            }
            MATELEM(B, i, j) = p_Copy(entry, Z);
            MATELEM(B, j, i) = p_Copy(entry, Z);
            idx++;
        }
    }
    std::cout << "[DEBUG] Computing B1 matrix" << std::endl;
    matrix zvars = buildZVars(Z, n, m2, pq);
    matrix A=U;
    matrix A_mapped = mp_Copy(A, Z);
    std::cout << "[DEBUG] Mapped A to Z = " << std::endl;
    std::cout << matrixToString(A_mapped, Z);
    matrix Bentries = mp_Mult(zvars, A_mapped, Z);
    std::cout << "[DEBUG] Bentries = zvars * A = " << std::endl;
    std::cout << matrixToString(Bentries, Z);
    matrix B1 = mpNew(nvars1, nvars1);
    for (int i = 0; i < nvars1; ++i)
        for (int j = 0; j < nvars1; ++j)
            MATELEM(B1, i + 1, j + 1) = p_Copy(Bentries->m[i * nvars1 + j], Z);
    B = mp_Add(B, B1, Z);
    std::cout << "[DEBUG] Final Baikov matrix B = " << std::endl;
    std::cout << matrixToString(B, Z);

    G.baikovover = Z;
    G.baikovmatrix = B;

    // Cleanup
    id_Delete(&PI_mapped, G.overpoly);
    mp_Delete(&A, G.overpoly);
    mp_Delete(&zvars, Z);
    mp_Delete(&Bentries, Z);
    mp_Delete(&B1, Z);
    mp_Delete(&pq, Z);
    p_Delete(&sumt, Z);
    for (int i = 0; i < mt + 1; ++i) omFree(extNames[i]);
    for (int i = 0; i < n; ++i) omFree(varNames[i]);
    omFree(extNames);
    omFree(varNames);
    omFree(perm);
    omFree(par_perm);

    rChangeCurrRing(savedRing);
    std::cout << "[DEBUG] computeBaikovMatrix complete" << std::endl;
    return G;
}
// feynmanDenominators: Compute the ideal containing the propagators in the Feynman integral
// Each propagator is the square of the label for internal edges (edges with 2 vertices)
ideal feynmanDenominators(const LabeledGraph& G)
{
    // Save current ring state
    ring savedRing = currRing;
    rChangeCurrRing(G.over);
    
    // Create an ideal to hold the propagators
    ideal J = idInit(G.labels->nr + 1, 1); // Initialize with enough space
    
    // Process each edge in the graph
    for (int i = 0; i <= G.labels->nr; i++) {
        // Check if this edge has 2 vertices (internal edge)
        if (i <= G.edges->nr && G.edges->m[i].rtyp == LIST_CMD) {
            lists edge = (lists)G.edges->m[i].Data();
            if (edge && edge->nr == 1) { // Internal edge has 2 vertices (nr=1 means 2 elements)
                // Get the label polynomial
                if (G.labels->m[i].rtyp == POLY_CMD) {
                    poly label = (poly)G.labels->m[i].Data();
                    if (label) {
                        // Square the label (L[i]^2) to create the propagator
                        poly propagator = p_Power(p_Copy(label, G.over), 2, G.over);
                        
                        // Add to the ideal at position i
                        J->m[i] = propagator;
                    }
                }
            }
        }
    }
    
    // Cleanup and resize the ideal
    idSkipZeroes(J);
    
    // Restore original ring state
    rChangeCurrRing(savedRing);
    
    return J;
}

