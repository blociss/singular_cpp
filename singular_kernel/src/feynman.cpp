#include "feynman.h"
#include <iostream>
#include <singular/Singular/libsingular.h>
#include "polys/ext_fields/transext.h"
#include <singular/polys/monomials/ring.h>
#include <singular/Singular/maps_ip.h>
#include <singular/Singular/ipid.h>
#include <singular/polys/simpleideals.h>
#include <singular/coeffs/numbers.h> // For n_SetMap
#include <singular/polys/prCopy.h>

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
    std::cout << "[DEBUG] Entering makeLabeledGraph\n";    LabeledGraph G;
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

// Remove j-th variable from a ring
ring removeVariable(ring R, int j) {
    if (j < 1 || j > rVar(R)) {
        std::cerr << "[ERROR] Index out of range in removeVariable." << std::endl;
        return R;
    }

    // Create new ring with one less variable
    char** newVarNames = (char**)omAlloc((rVar(R) - 1) * sizeof(char*));
    int newIdx = 0;
    
    for (int i = 0; i < rVar(R); i++) {
        if (i + 1 != j) {
            newVarNames[newIdx] = omStrDup(rRingVar(i, R));
            newIdx++;
        }
    }

    // Create new ring with same characteristics but fewer variables
    rRingOrder_t* orders = (rRingOrder_t*)omAlloc(2 * sizeof(rRingOrder_t));
    int* blocks = (int*)omAlloc(4 * sizeof(int));
    
    orders[0] = ringorder_lp;
    orders[1] = ringorder_C;
    blocks[0] = rVar(R) - 1;
    blocks[1] = 1;
    blocks[2] = 0;
    blocks[3] = 1;
    
    ring newR = rDefault(rChar(R), rVar(R) - 1, newVarNames, 2, orders, blocks, blocks + 2);
    rComplete(newR);

    // Free allocated memory
    for (int i = 0; i < rVar(R) - 1; i++) {
        omFree(newVarNames[i]);
    }
    omFree(newVarNames);
    omFree(orders);
    omFree(blocks);

    return newR;
}

// Remove eliminated variables from a labeled graph
void removeElimVars(LabeledGraph& G) {
    std::cout << "[DEBUG] Entering removeElimVars" << std::endl;
    if (!G.elimvars) {
        std::cout << "[DEBUG] No elimvars list, exiting" << std::endl;
        return;
    }
    
    std::cout << "[DEBUG] Counting variables to eliminate" << std::endl;
    // Count variables to eliminate
    int elimCount = 0;
    for (int i = 0; i <= G.elimvars->nr; i++) {
      //std::cout << "[DEBUG] Checking elimvar " << i << ", type: " << G.elimvars->m[i].rtyp << std::endl;
        if (G.elimvars->m[i].rtyp == POLY_CMD) {
            elimCount++;
            poly p = (poly)G.elimvars->m[i].Data();
            std::cout << "[DEBUG] Found polynomial to eliminate: " << p_String(p, currRing) << std::endl;
        }
    }
    
    if (elimCount == 0) {
        std::cout << "[DEBUG] No variables to eliminate" << std::endl;
        return;
    }
    
    std::cout << "[DEBUG] Found " << elimCount << " variables to eliminate" << std::endl;
    
    // Create new ring without eliminated variables
    ring oldRing = currRing;
    ring newRing = NULL;
    
    std::cout << "[DEBUG] Current ring has " << rVar(currRing) << " variables" << std::endl;
    
    // Build list of variables to eliminate
    int* varsToElim = (int*)omAlloc0(sizeof(int) * elimCount);
    int elimIdx = 0;
    
    for (int i = 0; i <= G.elimvars->nr; i++) {
        if (G.elimvars->m[i].rtyp != POLY_CMD) continue;
        
        poly p = (poly)G.elimvars->m[i].Data();
        int var = 1;
        for (int j = 1; j <= rVar(currRing); j++) {
            if (p_GetExp(p, j, currRing) > 0) {
                var = j;
                break;
            }
        }
        varsToElim[elimIdx++] = var;
    }
    
    // Sort variables in descending order to remove from highest to lowest
    for (int i = 0; i < elimCount - 1; i++) {
        for (int j = 0; j < elimCount - i - 1; j++) {
            if (varsToElim[j] < varsToElim[j + 1]) {
                int temp = varsToElim[j];
                varsToElim[j] = varsToElim[j + 1];
                varsToElim[j + 1] = temp;
            }
        }
    }
    
    // Remove variables one by one
    for (int i = 0; i < elimCount; i++) {
        newRing = removeVariable(currRing, varsToElim[i]);
        if (!newRing) {
            std::cout << "[ERROR] Failed to create new ring" << std::endl;
            omFree(varsToElim);
            return;
        }
        rChangeCurrRing(newRing);
    }
    
    omFree(varsToElim);
    
    //std::cout << "[DEBUG] Mapping labels to new ring" << std::endl;
    // Map labels to new ring
    if (!G.labels) {
        std::cout << "[ERROR] Labels list is null" << std::endl;
        return;
    }
    
    for (int i = 0; i <= G.labels->nr; i++) {
       // std::cout << "[DEBUG] Processing label " << i << std::endl;
        if (G.labels->m[i].rtyp != POLY_CMD) {
           // std::cout << "[DEBUG] Skipping non-polynomial label " << i << std::endl;
            continue;
        }
        
        poly p = (poly)G.labels->m[i].Data();
        if (!p) {
            //std::cout << "[DEBUG] Skipping null polynomial label " << i << std::endl;
            continue;
        }
        //std::cout << "[DEBUG] Original label: " << p_String(p, oldRing) << std::endl;
        
        // Create a new polynomial in the new ring
        poly newP = NULL;
        if (p_IsConstant(p, oldRing)) {
            newP = p_ISet(n_Int(pGetCoeff(p), oldRing->cf), currRing);
            G.labels->m[i].data = (void*)newP;
        } else {
            //std::cout << "[DEBUG] Label " << i << " mapped to zero" << std::endl;
            G.labels->m[i].data = NULL;
        }
    }
    
    std::cout << "[DEBUG] Switching back to old ring" << std::endl;
    rChangeCurrRing(oldRing);
    std::cout << "[DEBUG] Exiting removeElimVars" << std::endl;
    }



LabeledGraph labelGraph(Graph G, int ch) {
    int ct = 0;
    for (int i = 0; i <= G.edges->nr; i++) {
        lists edge = (lists)G.edges->m[i].data;
        if (edge->nr == 0) ct++;
    }
    int anzq = G.edges->nr + 1 - ct;

    std::cout << "[DEBUG] ct calculated: " << ct << "\n";
    std::cout << "[DEBUG] anzq calculated: " << anzq << "\n";

    // Create base coefficient field (rationals QQ)
    coeffs baseCoeff = nInitChar(n_Q, NULL);
    if (!baseCoeff) {
        std::cerr << "Failed to create base coefficient field QQ.\n";
        return LabeledGraph();
    }
    std::cout << "[DEBUG] Base coefficient field created (QQ).\n";

    // Create polynomial ring P: QQ[q(1..anzq), p(1..ct)], ip ordering
    std::vector<char*> varsP;
    for (int i = 1; i <= ct; i++)
        varsP.push_back(omStrDup(("p(" + std::to_string(i) + ")").c_str()));
    for (int i = 1; i <= anzq; i++)
        varsP.push_back(omStrDup(("q(" + std::to_string(i) + ")").c_str()));

    ring P = rDefault(baseCoeff, varsP.size(), varsP.data(), ringorder_ip);
    rComplete(P);
    rChangeCurrRing(P);
    std::cout << "[DEBUG] Ring P created: " << rString(P) << "\n";

    // Create extension coefficient field QQ(p(1),...,p(ct))
    std::vector<char*> varsCoeff;
    for (int i = 1; i <= ct; i++)
        varsCoeff.push_back(omStrDup(("p(" + std::to_string(i) + ")").c_str()));

    ring coeffRing = rDefault(baseCoeff, ct, varsCoeff.data(), ringorder_dp);
    rComplete(coeffRing);
    TransExtInfo param = {coeffRing};
    coeffs extCoeff = nInitChar(n_transExt, &param);
    if (!extCoeff) {
        std::cerr << "Failed to create extension coefficient field.\n";
        return LabeledGraph();
    }
    std::cout << "[DEBUG] Extension coefficient field created: QQ(p(1)..p(ct)).\n";

    // Create polynomial ring R: QQ(p(1),...,p(ct))[q(1..anzq)], ip ordering
    std::vector<char*> varsR;
    for (int i = 1; i <= anzq; i++)
        varsR.push_back(omStrDup(("q(" + std::to_string(i) + ")").c_str()));

    ring R = rDefault(extCoeff, anzq, varsR.data(), ringorder_ip);
    rComplete(R);
    std::cout << "[DEBUG] Ring R created: " << rString(R) << "\n";

    // Step: Create labels in P explicitly
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

    std::cout << "print labelsP\n";
    for (int i = 0; i < IDELEMS(labelsP); i++) {
        //std::cout << "[DEBUG] Label " << i << ": " << pString(labelsP->m[i]) << "\n";
    }
    std::cout << "[DEBUG] Created labels in P explicitly.\n";
    // Step: Map labels from P to R
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
        //std::cout << "[DEBUG] Label " << i << ": " << p_String((poly)labelsList->m[i].data, R) << "\n";
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

    if (var == 0) return p_Copy(label, r);

    // Check if label is equal to a, if so return b
    if (p_ComparePolys(label, a, r)) {
        return p_Copy(b, r);
    }

    poly result = p_SubstPoly(p_Copy(label, r), var, p_Copy(b, r), r, r, nMap);
    if (result) p_Normalize(result, r);
    return result;
}

LabeledGraph substituteGraph(LabeledGraph G, poly a, poly b) {
    ring savedRing = currRing;
    rChangeCurrRing(G.over);

    lists L = (lists)omAlloc0(sizeof(slists));
    L->Init(G.labels->nr + 1);
    L->nr = G.labels->nr;
    std::cout << "[DEBUG] G.labels:\n";
for(int i = 0; i <= G.labels->nr; i++){
    std::cout << "[" << i+1 << "] = " << pString((poly)G.labels->m[i].Data()) << "\n";
}
    for (int i = 0; i <= G.labels->nr; i++) {
        L->m[i].Init();
        L->m[i].rtyp = POLY_CMD;
        poly label = (poly)G.labels->m[i].Data();
        if (!label) {
            L->m[i].data = NULL;
            continue;
        }

        std::cout << "Substituting before: ";
        std::cout << pString(a);
        std::cout << " by ";
        std::cout << pString(b);
        std::cout << " in ";
        std::cout << pString(label);
        std::cout << "\n";

        poly result = substitutePoly(label, a, b, G.over);
        std::cout << "[DEBUG] after Substituted label[" << i+1 << "]:";
        std::cout << pString(result) << "\n";

        L->m[i].data = result;
    }

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
        std::cout << "[DEBUG] Ideal element I[" << i << "] = " << s << std::endl;
        omFree(s);
    }

    std::cout << "[DEBUG] Mapping ideal from G.over to G.overpoly" << std::endl;
    int nvars = rVar(G.over);
    int npars = rPar(G.over);
    std::cout << "[DEBUG] nvars = " << nvars << ", npars = " << npars << std::endl;
    int *perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
    int *par_perm = (int*)omAlloc0((npars + 1) * sizeof(int));

    for (int i = 1; i <= nvars; i++) perm[i] = i + npars;
    for (int i = 0; i < npars; i++) par_perm[i] = i + 1;

    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);
    ideal I_mapped = id_PermIdeal(I, 1, IDELEMS(I), perm, G.over, G.overpoly, nMap, par_perm, npars, FALSE);

    rChangeCurrRing(G.overpoly);
    si_opt_1 |= Sy_bit(OPT_REDTAIL);
    ideal I_std = kStd(I_mapped, NULL, testHomog, NULL, NULL, 0, TRUE);

    std::cout << "computing Std ideal" << std::endl;
    for (int i = 0; i < IDELEMS(I_std); i++) {
        std::cout << p_String(I_std->m[i], G.overpoly) << "," << std::endl;
    }

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
        std::cout << "[DEBUG] Skipping constant polynomial: " << p_String(f, G.overpoly) << std::endl;
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

// Compute propagators for a labeled graph
ideal propagators(const LabeledGraph& G) {
    ring R = G.over;
    ideal I = idInit(1, 1); // Initial empty ideal

    for (int i = 0; i < size(G.edges); i++) {
        lists edge = (lists)G.edges->m[i].Data();
        if (edge->nr == 1 && edge->m[1].rtyp == INT_CMD) { // Bounded edge (size=2)
            poly label = (poly)G.labels->m[i].Data();
            if (label) {
                // Create a temporary ideal for the polynomial
                ideal temp = idInit(1, 1);
                temp->m[0] = p_Copy(label, R);
                I = id_Add(I, temp, R); // Add ideals
                id_Delete(&temp, R); // Clean up temporary ideal
            }
        }
    }

    return I;
}

// Compute ISP (Internal Space of Propagators)
lists ISP(const LabeledGraph& G) {
    ideal P = propagators(G);
    int n = IDELEMS(P);
    lists result = (lists)omAlloc(sizeof(sleftv));
    result->Init(n);

    for (int i = 0; i < n; i++) {
        if (P->m[i]) {
            result->m[i].rtyp = POLY_CMD;
            result->m[i].data = p_Copy(P->m[i], G.over);
        }
    }
    result->nr = n - 1;

    id_Delete(&P, G.over);
    return result;
}


LabeledGraph computeBaikovMatrix(const Graph& G0) {
    // Convert graph to labeled graph
    LabeledGraph lG = makeLabeledGraph(G0.vertices, G0.edges);
    
    // Initialize labels with variables z_1, z_2, etc.
    lG.labels = (lists)omAlloc(sizeof(sleftv));
    lG.labels->Init(G0.edges->nr + 1);
    for (int i = 0; i <= G0.edges->nr; i++) {
        poly p = p_ISet(1, currRing);
        p_SetExp(p, i+1, 1, currRing); // Set exponent for z_{i+1}
        p_Setm(p, currRing);
        lG.labels->m[i].rtyp = POLY_CMD;
        lG.labels->m[i].data = (void*)p;
    }
    
    LabeledGraph G1 = eliminateVariables(lG);
    removeElimVars(G1);
    return computeBaikovMatrix(G1);
}

LabeledGraph computeBaikovMatrix(const LabeledGraph& G0) {
    std::cout << "[DEBUG] Entering computeBaikovMatrix" << std::endl;
    std::cout << "[DEBUG] Input graph: " << std::endl;
    printLabeledGraph(G0);
    std::cout << "[DEBUG] Starting computeBaikovMatrix" << std::endl;
    // Create a copy of the input graph
    LabeledGraph G1;
    G1.vertices = NULL;
    G1.edges = NULL;
    G1.labels = NULL;
    G1.over = NULL;
    G1.overpoly = NULL;
    G1.elimvars = NULL;
    G1.baikovover = NULL;
    G1.baikovmatrix = NULL;
    std::cout << "[DEBUG] Entering computeBaikovMatrix: Copying vertices" << std::endl;
    printLabeledGraph(G1);
    // Copy vertices
    if (G0.vertices) {
        G1.vertices = (lists)omAllocBin(slists_bin);
        if (!G1.vertices) {
            std::cout << "[ERROR] Failed to allocate memory for vertices" << std::endl;
            return G1;
        }
        G1.vertices->Init();
        G1.vertices->m = (leftv)omAlloc0((G0.vertices->nr + 1) * sizeof(sleftv));
        G1.vertices->nr = G0.vertices->nr;
        for (int i = 0; i <= G0.vertices->nr; i++) {
            G1.vertices->m[i].Init();
            G1.vertices->m[i].rtyp = G0.vertices->m[i].rtyp;
            G1.vertices->m[i].data = G0.vertices->m[i].data;
        }
    }
    
    // Copy edges
    if (G0.edges) {
        G1.edges = (lists)omAllocBin(slists_bin);
        if (!G1.edges) {
            std::cout << "[ERROR] Failed to allocate memory for edges" << std::endl;
            return G1;
        }
        G1.edges->Init();
        G1.edges->m = (leftv)omAlloc0((G0.edges->nr + 1) * sizeof(sleftv));
        G1.edges->nr = G0.edges->nr;
        for (int i = 0; i <= G0.edges->nr; i++) {
            G1.edges->m[i].Init();
            G1.edges->m[i].rtyp = G0.edges->m[i].rtyp;
            G1.edges->m[i].data = G0.edges->m[i].data;
        }
    }
    
    // Copy ring
    if (G0.over) {
        ring savedRing = currRing;
        rChangeCurrRing(G0.over);
        G1.over = rCopy(currRing);
        rChangeCurrRing(savedRing);
        if (!G1.over) {
            std::cout << "[ERROR] Failed to copy ring" << std::endl;
            return G1;
        }
    }
    
    // Copy labels
    if (G0.labels) {
        G1.labels = (lists)omAllocBin(slists_bin);
        if (!G1.labels) {
            std::cout << "[ERROR] Failed to allocate memory for labels" << std::endl;
            return G1;
        }
        G1.labels->Init();
        G1.labels->m = (leftv)omAlloc0((G0.labels->nr + 1) * sizeof(sleftv));
        G1.labels->nr = G0.labels->nr;
        //std::cout << "[DEBUG] Copying " << (G0.labels->nr + 1) << " labels" << std::endl;
        for (int i = 0; i <= G0.labels->nr; i++) {
            //std::cout << "[DEBUG] Copying label " << i << std::endl;
            G1.labels->m[i].Init();
            G1.labels->m[i].rtyp = G0.labels->m[i].rtyp;
            //std::cout << "[DEBUG] Label " << i << " type: " << G0.labels->m[i].rtyp << std::endl;
            if (G0.labels->m[i].rtyp == POLY_CMD) {
                ring savedRing = currRing;
                if (G0.over) {
                    rChangeCurrRing(G0.over);
                    G1.labels->m[i].data = p_Copy((poly)G0.labels->m[i].data, G0.over);
                    rChangeCurrRing(savedRing);
                } else {
                    std::cerr << "[ERROR] Source ring is null" << std::endl;
                    G1.labels->m[i].data = NULL;
                }
            } else {
                //std::cout << "[DEBUG] Label " << i << " is not a polynomial" << std::endl;
                G1.labels->m[i].data = G0.labels->m[i].data;
            }
        }
        std::cout << "[DEBUG] Labels copied successfully" << std::endl;
    } else {
        std::cout << "[DEBUG] No labels to copy" << std::endl;
    }
    
    // Copy polynomial and eliminated variables
    G1.overpoly = G0.overpoly;
    G1.elimvars = G0.elimvars;
    
    std::cout << "[DEBUG] Created copy of input graph" << std::endl;
    
    // Save current ring state
    ring savedRing = currRing;
    
    // Create the Baikov ring as a copy of G0's ring
    if (!G0.over) {
        std::cout << "[ERROR] Source ring is null" << std::endl;
        return G1;
    }
    
    // Switch to source ring and create Baikov ring
    rChangeCurrRing(G0.over);
    G1.baikovover = rCopy(currRing);
    
    if (!G1.baikovover) {
        std::cout << "[ERROR] Failed to create Baikov ring" << std::endl;
        rChangeCurrRing(savedRing);
        return G1;
    }
    
    // Switch to Baikov ring for matrix operations
    rChangeCurrRing(G1.baikovover);
    
    // Initialize the Baikov matrix
    int n = size(G0.edges);
    std::cout << "[DEBUG] Creating Baikov matrix of size " << n << "x" << n << std::endl;
    G1.baikovmatrix = mpNew(n, n);
    
    if (!G1.baikovmatrix) {
        std::cout << "[ERROR] Failed to create Baikov matrix" << std::endl;
        rChangeCurrRing(savedRing);
        return G1;
    }
    
    // Fill the matrix with zeros first
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            poly p = p_ISet(0, G1.baikovover);
            if (!p) {
                std::cout << "[ERROR] Failed to create zero polynomial at (" << i << "," << j << ")" << std::endl;
                continue;
            }
            MATELEM(G1.baikovmatrix, i, j) = p;
            std::cout << "[DEBUG] Set (" << i << "," << j << ") to zero" << std::endl;
        }
    }
    
    // Compute the Baikov matrix entries
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (G0.edges->m[i].rtyp != LIST_CMD || G0.edges->m[j].rtyp != LIST_CMD) continue;
            
            lists edge1 = (lists)G0.edges->m[i].Data();
            lists edge2 = (lists)G0.edges->m[j].Data();
            if (!edge1 || !edge2) continue;
            
            poly p = NULL;
            
            // Handle unbounded edges (single vertex)
            bool edge1_unbounded = (edge1->nr == 0);
            bool edge2_unbounded = (edge2->nr == 0);
            
            if (edge1_unbounded || edge2_unbounded) {
                if (edge1->m[0].rtyp != INT_CMD || edge2->m[0].rtyp != INT_CMD) continue;
                
                int v1 = (int)(long)edge1->m[0].Data();
                int v2 = (int)(long)edge2->m[0].Data();
                
                if (v1 == v2) {
                    p = p_ISet(1, G1.baikovover);
                }
            } else {
                if (edge1->m[0].rtyp != INT_CMD || edge1->m[1].rtyp != INT_CMD ||
                    edge2->m[0].rtyp != INT_CMD || edge2->m[1].rtyp != INT_CMD) continue;
                
                int v1 = (int)(long)edge1->m[0].Data();
                int v2 = (int)(long)edge1->m[1].Data();
                int w1 = (int)(long)edge2->m[0].Data();
                int w2 = (int)(long)edge2->m[1].Data();
                
                if ((v1 == w1 && v2 == w2) || (v1 == w2 && v2 == w1)) {
                    p = p_ISet(1, G1.baikovover);
                } else if (v1 == w1 || v1 == w2 || v2 == w1 || v2 == w2) {
                    if (G0.labels && i <= G0.labels->nr && j <= G0.labels->nr &&
                        G0.labels->m[i].rtyp == POLY_CMD && G0.labels->m[j].rtyp == POLY_CMD) {
                        
                        std::cout << "[DEBUG] Processing labels for edges " << i << " and " << j << std::endl;
                        poly label1 = (poly)G0.labels->m[i].Data();
                        poly label2 = (poly)G0.labels->m[j].Data();
                        std::cout << "[DEBUG] Label1: " << (label1 ? p_String(label1, G0.over) : "null") << std::endl;
                        std::cout << "[DEBUG] Label2: " << (label2 ? p_String(label2, G0.over) : "null") << std::endl;
                        
                        if (label1 && label2) {
                            // We're already in G1.baikovover's context
                            p = pp_Mult_qq(p_Copy(label1, G1.baikovover), p_Copy(label2, G1.baikovover), G1.baikovover);
                            char* pStr = p_String(p, G1.baikovover);
                            std::cout << "[DEBUG] Product: " << (pStr ? pStr : "null") << std::endl;
                            if (pStr) omFree(pStr);
                        }
                    }
                }
            }
            
            if (p) {
                MATELEM(G1.baikovmatrix, i + 1, j + 1) = p;
                if (i != j) {
                    MATELEM(G1.baikovmatrix, j + 1, i + 1) = p_Copy(p, G1.baikovover);
                }
            }
        }
    }
    
    // Restore original ring state
    rChangeCurrRing(savedRing);
    
    std::cout << "[DEBUG] Exiting computeBaikovMatrix" << std::endl;
    return G1;
}