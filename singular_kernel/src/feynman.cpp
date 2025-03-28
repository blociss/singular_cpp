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
#include <kernel/combinatorics/stairc.h> // For scKBase
#include <sstream>

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
    // Count unbounded edges (ct) and bounded edges (anzq)
    int ct = 0;
    int anzq = 0;
    for (int i = 0; i <= G.edges->nr; i++) {
        lists edge = (lists)G.edges->m[i].data;
        if (edge->nr == 0) { // Unbounded edge (one vertex)
            ct++;
        } else if (edge->nr == 1) { // Bounded edge (two vertices)
            anzq++;
        }
    }

    std::cout << "[DEBUG] ct (unbounded edges) = " << ct << "\n";
    std::cout << "[DEBUG] anzq (bounded edges) = " << anzq << "\n";

    // Create base coefficient field (rationals QQ)
    coeffs baseCoeff = nInitChar(n_Q, NULL);
    if (!baseCoeff) {
        std::cerr << "Failed to create base coefficient field QQ.\n";
        return LabeledGraph();
    }
    std::cout << "[DEBUG] Base coefficient field created (QQ).\n";

    // Create polynomial ring P: QQ[p(1..ct),q(1..anzq)], ip ordering
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
void printElimVarsOnly(const lists elimvars, ring r) {
    if (!elimvars) {
        std::cout << "[DEBUG] elimvars is NULL" << std::endl;
        return;
    }

    ring savedRing = currRing;
    rChangeCurrRing(r);

    std::cout << "\nElimination Variables:\n";

    int nr = elimvars->nr;
    for (int i = 0; i <= nr; ++i) {
        sleftv* lv = &(elimvars->m[i]);

        std::cout << "[DEBUG] elimvar " << i << ": ";

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
            std::cout << "NULL polynomial" << std::endl;
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
// Deep copy of a LabeledGraph structure
LabeledGraph deepCopyLabeledGraph(const LabeledGraph& G0) {
    LabeledGraph G1;
    G1.vertices = lCopy(G0.vertices);
    G1.edges = lCopy(G0.edges);
    G1.elimvars = lCopy(G0.elimvars);

    if (G0.labels) {
        G1.labels = (lists)omAllocBin(slists_bin);
        G1.labels->Init(G0.labels->nr + 1);
        for (int i = 0; i <= G0.labels->nr; ++i) {
            G1.labels->m[i].Init();
            G1.labels->m[i].rtyp = G0.labels->m[i].rtyp;
            if (G0.labels->m[i].rtyp == POLY_CMD && G0.labels->m[i].data) {
                G1.labels->m[i].data = p_Copy((poly)G0.labels->m[i].data, G0.over);
            } else {
                G1.labels->m[i].data = G0.labels->m[i].data;
            }
        }
    } else {
        G1.labels = nullptr;
    }

    G1.over = G0.over;
    G1.overpoly = G0.overpoly;
    G1.baikovover = G0.baikovover;
    G1.baikovmatrix = G0.baikovmatrix;

    return G1;
}

ideal buildGramMatrix (ring R, const ring RP) {
    rChangeCurrRing(R);

    int npars = rPar(R);
    int startvars = npars + 1;

    rChangeCurrRing(RP);
    int nvars = rVar(RP);
    int totalEntries = nvars * nvars;

    std::cout << "[DEBUG] startvars = " << startvars << std::endl;
    std::cout << "[DEBUG] nvars(RP) = " << nvars << std::endl;

    ideal gram = idInit(totalEntries, 1);
    int idx = 0;

    for (int i = 1; i <= nvars; ++i) {
        for (int j = 1; j <= nvars; ++j) {
            if (i >= startvars || j >= startvars) {
                poly p = p_One(RP);
                p_SetExp(p, i, 1, RP);
                p_SetExp(p, j, 1, RP);
                p_Setm(p, RP);
                gram->m[idx] = p;
                char* str = p_String(p, RP);
                std::cout << "[DEBUG] gram[" << (idx + 1) << "] = " << str << std::endl;
                omFree(str);
            } else {
                gram->m[idx] = p_ISet(0, RP); // Zero polynomial
            }
            idx++;  // This should be outside the if-else block
        }
    }

    std::cout << "[DEBUG] Gram matrix entries:" << std::endl;
    for (int i = 0; i < totalEntries; ++i) {
        char* s = p_String(gram->m[i], RP);
        std::cout << "[" << (i + 1) << "]: " << s << std::endl;
        omFree(s);
    }

    return gram;
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

LabeledGraph computeBaikovMatrix(const LabeledGraph& G0) {
    std::cout << "[DEBUG] computeBaikovMatrix started" << std::endl;

    // Step 1: Copy the input labeled graph
    std::cout << "[DEBUG] Deep copying labeled graph" << std::endl;
    //LabeledGraph G = deepCopyLabeledGraph(G0);
    LabeledGraph G = G0;
    printLabeledGraph(G);
    // Step 2: Compute propagators and ISP
    rChangeCurrRing(G.over);
    std::cout << "[DEBUG] Computing propagators P" << std::endl;
    ideal P = propagators(G);
    printIdeal(P);
    std::cout << "[DEBUG] Computing ISP" << std::endl;
    ideal I = ISP(G);
    printIdeal(I);
    std::cout << "[DEBUG] Combining P and I to PI" << std::endl;
    ideal PI = id_Add(P, I, G.over);
    printIdeal(PI);
    id_Delete(&P, G.over);
    id_Delete(&I, G.over);

    // Step 3: Map PI to overpoly ring
    rChangeCurrRing(G.overpoly);
    int nvars = rVar(G.over);
    int npars = rPar(G.over);
    int* perm = (int*)omAlloc0((nvars + 1) * sizeof(int));
    int* par_perm = (int*)omAlloc0((npars + 1) * sizeof(int));
    for (int i = 1; i <= nvars; ++i) perm[i] = i + npars;
    for (int i = 0; i < npars; ++i) par_perm[i] = i + 1;
    nMapFunc nMap = n_SetMap(G.over->cf, G.overpoly->cf);
    std::cout << "[DEBUG] Mapping PI to RP=G.overpoly ring" << std::endl;
    ideal PI_mapped = id_PermIdeal(PI, 1, IDELEMS(PI), perm, G.over, G.overpoly, nMap, par_perm, npars, FALSE);
    id_Delete(&PI, G.over);

    // Step 4: Build Gram matrix    
    std::cout << "[DEBUG] Building Gram matrix" << std::endl;
std::cout << "[DEBUG] Gram matrix entries:" << std::endl;

ring R = G.over;
ring RP = G.overpoly;
std::cout<<" The graph G0 = ****"<<std::endl;
printLabeledGraph(G0);
std::cout<<" The graph G = ****"<<std::endl;
printLabeledGraph(G);

std::cout<<"ring RP = G.overpoly; = " << rString(RP) << std::endl;
std::cout<<"ring R = G.over; = " << rString(R) << std::endl;
    int idx = 0;
    int nvars1 = rVar(G.overpoly);
    std::cout << "[DEBUG] nvars1 =rVar(G.overpoly) = " << nvars1 << std::endl;

    int startvars = npars + 1;
    std::cout << "[DEBUG] startvars =rPar(G.over) + 1 = " << startvars << std::endl;

    ideal gram = idInit(3, 1); // Start small and resize dynamically

    for (int i = 1; i <= nvars1; i++) {
        std::cout << "[DEBUG] i = " << i << std::endl;
        for (int j = 1; j <= nvars1; j++) {
            std::cout << "[DEBUG] j = " << j << std::endl;
            if (idx >= IDELEMS(gram)) {
                int newSize = idx + 1;
                ideal tmpL = idInit(newSize, 1);
                for (int k = 0; k < idx; ++k) {
                    tmpL->m[k] = gram->m[k];
                    gram->m[k] = NULL;
                }
                idDelete(&gram);
                gram = tmpL;
            }
            std::cout << "[DEBug] i= " << i << ", j= " << j << ", idx= " << idx << std::endl;
            if (i >= startvars || j >= startvars) {
                poly p = p_One(RP);
                p_SetExp(p, i, 1, RP);
                p_SetExp(p, j, 1, RP);
                p_Setm(p, RP);
                gram->m[idx] = p;
                std::cout << "[DEBUG] gram[" << (idx + 1) << "] = " << p_String(p, RP) << std::endl;
            } else {
                gram->m[idx] = NULL;
                std::cout << "[DEBUG] gram[" << (idx + 1) << "] = 0" << std::endl;
            }
            idx++;
        }
    }


std::cout << "[DEBUG] Gram matrix entries:" << std::endl;
printIdeal(gram);

    // Step 5: Add scalar products var(i)*var(j) for 1 <= i < j < startvars
    std::cout << "[DEBUG] Adding scalar products to PI_mapped" << std::endl;
    for (int i = 1; i < npars; ++i) {
        for (int j = i + 1; j < npars; ++j) {
            poly prod = p_One(G.overpoly);
            p_SetExp(prod, i, 1, G.overpoly);
            p_SetExp(prod, j, 1, G.overpoly);
            p_Setm(prod, G.overpoly);
            ideal temp = idInit(1,1);
            temp->m[0] = prod;
            PI_mapped = id_Add(PI_mapped, temp, G.overpoly);
            id_Delete(&temp, G.overpoly);
        }
    }

    // Step 6: Compute the lift matrix
    std::cout << "[DEBUG] Computing lift matrix" << std::endl;
    //ideal gramIdeal = id_Matrix2Module(mp_Copy(gram,currRing),currRing);
    ideal A_ideal = idLift(PI_mapped, gram, NULL, FALSE, FALSE, TRUE, NULL);
    matrix A = id_Module2Matrix(A_ideal,currRing);
    //id_Delete(&gramIdeal, currRing);
    id_Delete(&A_ideal, currRing);

    // Step 7: Construct Baikov ring Z
   std::cout << "[DEBUG] Constructing Baikov ring..." << std::endl;
 int m = npars;
    int m2 = (m * (m - 1)) / 2;
    int mt = m2 - 1;
    int n = IDELEMS(PI_mapped) - m2;
int extCount = mt + 1;   // t(1..mt) + D
int varCount = n;        // z(1..n)

char **extNames = (char**)omAlloc0(extCount * sizeof(char*));
for (int i = 0; i < mt; ++i) {
    std::string tname = "t(" + std::to_string(i + 1) + ")";
    extNames[i] = omStrDup(tname.c_str());
}
extNames[mt] = omStrDup("D");

char **varNames = (char**)omAlloc0(varCount * sizeof(char*));
for (int i = 0; i < varCount; ++i) {
    std::string zname = "z(" + std::to_string(i + 1) + ")";
    varNames[i] = omStrDup(zname.c_str());
}

ring Z = createRing(extNames, extCount, varNames, varCount, ringorder_dp);



for (int i = 0; i < 3; ++i) omFree(extNames[i]);
for (int i = 0; i < n + m2; ++i) omFree(varNames[i]);
omFree(extNames);
omFree(varNames);

    // Step 8: Build symmetric matrix B
    std::cout << "[DEBUG] Building symmetric matrix B" << std::endl;
    matrix B = mpNew(m, m);
    matrix pq = mpNew(1, m2);
    poly sumt = NULL;
    //int idx = 0;
    for (int i = 1; i <= m; ++i) {
        for (int j = i + 1; j <= m; ++j) {
            poly entry = NULL;
            if (idx <= mt) {
                entry = p_One(Z);
                p_SetExp(entry, idx, 1, Z);
                p_Setm(entry, Z);
                entry = p_Mult_nn(entry, n_Div(n_Init(1, Z->cf), n_Init(2, Z->cf), Z->cf), Z);
                pq->m[0 * m2 + idx - 1] = p_Copy(entry, Z);
                sumt = p_Add_q(sumt, p_Copy(entry, Z), Z);
            } else {
                entry = p_Neg(sumt, Z);
                pq->m[0 * m2 + idx - 1] = p_Copy(entry, Z);
            }
            MATELEM(B, i, j) = p_Copy(entry, Z);
            MATELEM(B, j, i) = p_Copy(entry, Z);
            idx++;
        }
    }

    // Step 9: Build zvars and compute B1 = zvars * A
    std::cout << "[DEBUG] Computing B1 matrix from zvars * A" << std::endl;
    matrix zvars = buildZVars(Z, n, m2, pq);
    ideal A_mapped_ideal = id_Matrix2Module(mp_Copy(A,Z),Z);
    ideal A_mapped = id_PermIdeal(A_mapped_ideal, 1, IDELEMS(A_mapped_ideal), perm, G.overpoly, Z, nMap, par_perm, npars, FALSE);
    matrix Bentries = mp_Mult(zvars, id_Module2Matrix(A_mapped, Z), Z);
    matrix B1 = mpNew(m, m);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            MATELEM(B1, i + 1, j + 1) = p_Copy(Bentries->m[i * m + j], Z);

    std::cout << "[DEBUG] Adding B1 to B" << std::endl;
    B = mp_Add(B, B1, Z);

    // Step 10: Store result in graph
    std::cout << "[DEBUG] Storing results in labeled graph" << std::endl;
    G.baikovover = Z;
    G.baikovmatrix = B;

    // Cleanup
    id_Delete(&PI_mapped, G.overpoly);
    mp_Delete(&A, G.overpoly);
    id_Delete(&A_mapped, Z);
    mp_Delete(&zvars, Z);
    mp_Delete(&Bentries, Z);
    mp_Delete(&B1, Z);
    mp_Delete(&pq, Z);
    omFree(perm);
    omFree(par_perm);

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
            std::cout << "  [" << i << "]: " << (pStr ? pStr : "null") << std::endl;
            if (pStr) omFree(pStr);
        }
    }
    
    rChangeCurrRing(savedRing);
}

// Add helper function to convert ideal to matrix
matrix idealToMatrix(ideal I) {
    int numElements = IDELEMS(I);
    matrix M = mpNew(numElements, 1);
    for(int i = 0; i < numElements; i++) {
        MATELEM(M, i+1, 1) = p_Copy(I->m[i], currRing);
    }
    return M;
}