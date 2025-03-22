#include "feynman.h"
#include <iostream>



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
    std::cout << "[DEBUG] Entering netList\n";
    Net N;
    N.rows = (lists)omAlloc(sizeof(sleftv));
    if (!N.rows) {
        std::cerr << "Memory allocation failed for N.rows\n";
        exit(1);
    }
    N.rows->Init(1);
    std::string result = "[";
    int listSize = L1 ? L1->nr + 1 : 0;
    //std::cout << "[DEBUG] netList: listSize=" << listSize << "\n";

    for (int j = 0; j < listSize; j++) {
        if (L1->m[j].rtyp != LIST_CMD) {
            std::cerr << "Expected list type at index " << j << "\n";
            continue;
        }
        //std::cout << "[DEBUG] netList: Processing element " << j << "\n";
        lists subList = (lists)L1->m[j].Data();
        if (!subList) {
            std::cerr << "Null sublist at index " << j << "\n";
            continue;
        }
        int subSize = subList->nr + 1;
        //std::cout << "[DEBUG] netList: subSize=" << subSize << "\n";

        std::string subResult = "[";
        for (int k = 0; k < subSize; k++) {
            //std::cout << "[DEBUG] netList: Processing sub-element " << k << "\n";
            if (subList->m[k].rtyp != INT_CMD) {
                std::cerr << "Expected integer type at subindex " << k << "\n";
                continue;
            }
            long intValue = (long)subList->m[k].Data();
            //std::cout << "[DEBUG] netList: value=" << intValue << "\n";
            subResult += std::to_string(intValue);
            if (k < subSize - 1) subResult += ",";
        }
        subResult += "]";
        result += subResult;
        if (j < listSize - 1) result += ", ";
    }
    result += "]";
    N.rows->m[0].rtyp = STRING_CMD;
    N.rows->m[0].data = (void*)omStrDup(result.c_str());
    N.rows->nr = 0;
    std::cout << "[DEBUG] Exiting netList\n";
    return N;
}

// printNet: Print Net rows
void printNet(const Net& N) {
    std::cout << "[DEBUG] Entering printNet\n";
    lists L = N.rows;
    int listSize = L ? L->nr + 1 : 0;
    //std::cout << "[DEBUG] printNet: listSize=" << listSize << "\n";
    for (int j = 0; j < listSize; j++) {
        //std::cout << "[DEBUG] printNet: Printing row " << j << "\n";
        std::cout << (char*)L->m[j].Data() << std::endl;
    }
    std::cout << "[DEBUG] Exiting printNet\n";
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

// makeLabeledGraph: Create a labeled graph
LabeledGraph makeLabeledGraph(lists vertices, lists edges, ring R, lists labels) {
    std::cout << "[DEBUG] Entering makeLabeledGraph\n";
    LabeledGraph G;
    G.vertices = vertices;
    G.edges = edges;
    G.over = R;
    G.labels = labels;
    std::cout << "[DEBUG] Exiting makeLabeledGraph\n";
    return G;
}

// printGraph: Print graph details
void printGraph(Graph G) {
    std::cout << "[DEBUG] Entering printGraph\n";
    Net edgeNet = netList(G.edges);
    printNet(edgeNet);

    int ct = 0;
    int edgesSize = G.edges ? G.edges->nr + 1 : 0;
    std::cout << "[DEBUG] printGraph: edgesSize=" << edgesSize << "\n";
    for (int i = 0; i < edgesSize; i++) {
        std::cout << "[DEBUG] printGraph: Checking edge " << i << "\n";
        lists edge = (lists)G.edges->m[i].Data();
        int edgeSize = edge ? edge->nr + 1 : 0;
        std::cout << "[DEBUG] printGraph: edgeSize=" << edgeSize << "\n";
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

    if (edgeNet.rows->m[0].data) omFree(edgeNet.rows->m[0].data);
    omFree(edgeNet.rows);
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
    std::cout << "[DEBUG] Entering createEdgeList\n";
    lists L = (lists)omAlloc(sizeof(sleftv));
    L->Init(size + singleSize);

    for (int i = 0; i < size; i++) {
        lists edge = (lists)omAlloc(sizeof(sleftv));
        edge->Init(2);
        edge->m[0].rtyp = INT_CMD;
        edge->m[0].data = (void*)(long)edges[i][0];
        edge->m[1].rtyp = INT_CMD;
        edge->m[1].data = (void*)(long)edges[i][1];
        edge->nr = 1;
        L->m[i].rtyp = LIST_CMD;
        L->m[i].data = (void*)edge;
    }

    for (int i = 0; i < singleSize; i++) {
        lists edge = (lists)omAlloc(sizeof(sleftv));
        edge->Init(1);
        edge->m[0].rtyp = INT_CMD;
        edge->m[0].data = (void*)(long)singleEdges[i];
        edge->nr = 0;
        L->m[size + i].rtyp = LIST_CMD;
        L->m[size + i].data = (void*)edge;
    }
    L->nr = size + singleSize - 1;
    std::cout << "[DEBUG] Exiting createEdgeList\n";
    return L;
}

// printLabeledGraph: Print labeled graph details
void printLabeledGraph(LabeledGraph G) {
    std::cout << "[DEBUG] Entering printLabeledGraph\n";
    
    // Print edges
    Net edgeNet = netList(G.edges);
    printNet(edgeNet);

    // Count unbounded edges
    int ct = 0;
    int edgesSize = G.edges ? G.edges->nr + 1 : 0;
    for (int i = 0; i < edgesSize; i++) {
        lists edge = (lists)G.edges->m[i].Data();
        if (edge && edge->nr == 0) ct++;
    }

    // Print graph summary
    if (ct != 0) {
        std::cout << "Graph with " << (G.vertices ? G.vertices->nr + 1 : 0) << " vertices, "
                  << (edgesSize - ct) << " bounded edges and " << ct << " unbounded edges" << std::endl;
    } else {
        std::cout << "Graph with " << (G.vertices ? G.vertices->nr + 1 : 0) << " vertices and "
                  << edgesSize << " edges" << std::endl;
    }

    // Print edge terms
    std::cout << "Edgeterms:" << std::endl;
    for (int i = 0; i < edgesSize; i++) {
        lists edge = (lists)G.edges->m[i].Data();
        if (!edge) continue;

        // Create edge representation
        std::string edgeStr = "[";
        for (int j = 0; j <= edge->nr; j++) {
            if (edge->m[j].rtyp == INT_CMD) {
                edgeStr += std::to_string((long)edge->m[j].Data());
                if (j < edge->nr) edgeStr += ",";
            }
        }
        edgeStr += "] => ";

        // Add label
        if (G.labels && i < G.labels->nr + 1) {
            char* label = p_String((poly)G.labels->m[i].Data(), G.over);
            edgeStr += label;
            omFree(label);
        }

        std::cout << edgeStr << std::endl;
    }

    std::cout << "[DEBUG] Exiting printLabeledGraph\n";
}



// Helper function to get size of lists
inline int getSize(lists L) {
    return L ? L->nr + 1 : 0;
}

// Helper function to create a Net from a string
Net net(const std::string& str) {
    Net N;
    N.rows = (lists)omAlloc(sizeof(slists));
    N.rows->Init(1);
    N.rows->m[0].rtyp = STRING_CMD;
    N.rows->m[0].data = omStrDup(str.c_str());
    return N;
}

// Helper function to concatenate Nets
Net catNet(const Net& N, const Net& M) {
    lists L = (lists)omAlloc(sizeof(slists));
    lists LN = N.rows;
    lists LM = M.rows;
    int widthN = getSize(LN);
    int widthM = getSize(LM);
    int nm = std::max(widthN, widthM);
    
    L->Init(nm);
    for (int j = 0; j < nm; j++) {
        if (j >= getSize(LN)) {
            // Create empty string of width N
            L->m[j].data = omStrDup("");
        } else if (j >= getSize(LM)) {
            // Create empty string of width M
            L->m[j].data = omStrDup("");
        } else {
            // Concatenate strings
            std::string s1((char*)LN->m[j].Data());
            std::string s2((char*)LM->m[j].Data());
            L->m[j].data = omStrDup((s1 + s2).c_str());
        }
        L->m[j].rtyp = STRING_CMD;
    }
    
    Net NM;
    NM.rows = L;
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
    return G;
}



// Balance ideal for a graph
ideal balancingIdeal(const LabeledGraph& G) {
    ideal I = idInit(1, 1);
    poly edg = NULL;
    poly rel = NULL;
    for (int i = 0; i <= G.vertices->nr; i++) {
        edg = NULL;
        for (int j = 0; j <= G.edges->nr; j++) {
            lists edge = (lists)G.edges->m[j].Data();
            poly label = (poly)G.labels->m[j].Data();
            if (edge->nr == 1 && (long)edge->m[0].Data() == i + 1) {
                edg = p_Add_q(edg, p_Copy(label, currRing), currRing);
            } else if (edge->nr == 1) {
                continue;
            } else {
                if ((long)edge->m[0].Data() == i + 1)
                    edg = p_Add_q(edg, p_Copy(label, currRing), currRing);
                if ((long)edge->m[1].Data() == i + 1)
                    edg = p_Add_q(edg, p_Neg(p_Copy(label, currRing), currRing), currRing);
            }
        }
        std::cout << "[DEBUG] edg: " << p_String(edg, currRing) << std::endl;
        ideal newI = idInit(1, 1);
        newI->m[0] = edg;
        I = idAdd(I, newI);
    }

    for (int j = 0; j <= G.edges->nr; j++) {
        lists edge = (lists)G.edges->m[j].Data();
        if (edge->nr == 0) continue;
        if (edge->nr == 1) {
            rel = p_Add_q(rel, p_Copy((poly)G.labels->m[j].Data(), currRing), currRing);
        }
    }
    if (rel != NULL) {
        ideal relI = idInit(1, 1);
        relI->m[0] = rel;
        I = idAdd(I, relI);
    }
    std::cout << "[DEBUG] I print " << std::endl;

    for (int i = 0; i < IDELEMS(I); i++) {
        std::cout<<"[DEBUG] I->m["<<i<<"]="<<pString((poly)I->m[i])<<std::endl;
    }
    return I;
}

// Substitute one polynomial for another in a labeled graph
LabeledGraph substituteGraph(LabeledGraph G, poly lhs, poly rhs) {
    for (int i = 0; i <= G.labels->nr; i++) {
        poly lab = (poly)G.labels->m[i].Data();
        // Find the leading variable of lhs
        int var = p_GetExp(lhs, 1, currRing);
        for (int j = 2; j <= rVar(currRing); j++) {
            if (p_GetExp(lhs, j, currRing) > 0) {
                var = j;
                break;
            }
        }
        poly newlab = p_Subst(lab, var, rhs, currRing);
        G.labels->m[i].data = (void*)newlab;
    }
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
    ring newR = rDefault(rChar(R), rVar(R) - 1, newVarNames);
    rComplete(newR);

    // Free allocated memory
    for (int i = 0; i < rVar(R) - 1; i++) {
        omFree(newVarNames[i]);
    }
    omFree(newVarNames);

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
    
    for (int i = 0; i <= G.elimvars->nr; i++) {
        if (G.elimvars->m[i].rtyp != POLY_CMD) {
          //  std::cout << "[DEBUG] Skipping non-polynomial elimvar " << i << std::endl;
            continue;
        }
        
        //std::cout << "[DEBUG] Processing elimvar " << i << std::endl;
        poly p = (poly)G.elimvars->m[i].Data();
        //std::cout << "[DEBUG] Got polynomial: " << p_String(p, currRing) << std::endl;
        
        int var = p_GetExp(p, 1, currRing);
        for (int j = 2; j <= rVar(currRing); j++) {
            if (p_GetExp(p, j, currRing) > 0) {
                var = j;
                //std::cout << "[DEBUG] Found variable to eliminate: " << var << std::endl;
                break;
            }
        }
        
        //std::cout << "[DEBUG] Removing variable " << var << std::endl;
        newRing = removeVariable(currRing, var);
        if (!newRing) {
            std::cout << "[ERROR] Failed to create new ring" << std::endl;
            return;
        }
        //std::cout << "[DEBUG] Switching to new ring with " << rVar(newRing) << " variables" << std::endl;
        rChangeCurrRing(newRing);
    }
    
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
        poly newP = p_Init(currRing);
        for (int j = 1; j <= rVar(oldRing); j++) {
            int exp = p_GetExp(p, j, oldRing);
            if (exp > 0) {
                p_SetExp(newP, j, exp, currRing);
            }
        }
        p_Setm(newP, currRing);
        
        if (!newP || n_IsZero((number)newP->coef, currRing->cf)) {
            std::cout << "[DEBUG] Label " << i << " mapped to zero" << std::endl;
            p_Delete(&newP, currRing);
            continue;
        }
        
        G.labels->m[i].data = (void*)newP;
        std::cout << "[DEBUG] New label: " << p_String(newP, currRing) << std::endl;
    }
    
    std::cout << "[DEBUG] Switching back to old ring" << std::endl;
    rChangeCurrRing(oldRing);
    std::cout << "[DEBUG] Exiting removeElimVars" << std::endl;
}

// Eliminate variables from a labeled graph using balancing equations
LabeledGraph eliminateVariables(LabeledGraph G) {
    std::cout << "[DEBUG] Entering eliminateVariables" << std::endl;
    ring R = currRing;
    //std::cout << "[DEBUG] Getting balancing ideal" << std::endl;
    ideal I = balancingIdeal(G);

     //std::cout << "[DEBUG] Creating empty ideal for quotient" << std::endl;
    ideal Q = idInit(1, 1); // Empty ideal for quotient
    tHomog h = testHomog;
    //std::cout << "[DEBUG] Computing standard basis" << std::endl;
    ideal I_std = kStd(I, Q, h, nullptr);
    //std::cout << "[DEBUG] Standard basis has " << IDELEMS(I_std) << " elements" << std::endl;

    LabeledGraph G1 = G;
    lists eliminatedVars = (lists)omAlloc(sizeof(sleftv));
    eliminatedVars->Init(IDELEMS(I_std));
    //std::cout << "[DEBUG] Initialized eliminatedVars with size " << IDELEMS(I_std) << std::endl;

    // We'll only process the first few elements to avoid memory issues
    int maxElements = std::min(IDELEMS(I_std), 3);
    for (int i = 0; i < maxElements; i++) {
        if (!I_std->m[i]) {
            //std::cout << "[DEBUG] Skipping null element " << i << std::endl;
            continue;
        }

        //std::cout << "[DEBUG] Processing element " << i << std::endl;
        poly ld = p_Head(I_std->m[i], currRing);
        if (!ld) {
            //std::cout << "[DEBUG] Skipping element with null leading term" << std::endl;
            continue;
        }
       // std::cout << "[DEBUG] Got leading term: " << p_String(ld, currRing) << std::endl;
        
        poly ta = p_Sub(p_Copy(I_std->m[i], currRing), p_Copy(ld, currRing), currRing);
        if (!ta) {
            //std::cout << "[DEBUG] Tail is null, skipping" << std::endl;
            p_Delete(&ld, currRing);
            continue;
        }
       //std::cout << "[DEBUG] Computed tail: " << p_String(ta, currRing) << std::endl;

        ideal tmpI = idInit(1, 1);
        tmpI->m[0] = p_Copy(ld, currRing);
        poly ld_orig = p_Copy(tmpI->m[0], currRing);

        tmpI->m[0] = p_Copy(ta, currRing);
        poly ta_orig = p_Copy(tmpI->m[0], currRing);

        eliminatedVars->m[i].rtyp = POLY_CMD;
        eliminatedVars->m[i].data = (void*)ld_orig;
        //std::cout << "[DEBUG] Set eliminated var " << i << ": " << p_String(ld_orig, currRing) << std::endl;

        //std::cout << "[DEBUG] Substituting in graph" << std::endl;
        G1 = substituteGraph(G1, ld_orig, ta_orig);

        p_Delete(&ld, currRing);
        p_Delete(&ta, currRing);
        id_Delete(&tmpI, currRing);
    }

    //std::cout << "[DEBUG] Cleaning up old labels" << std::endl;
    // Clean up old labels
    if (G1.labels) {
        G1.labels->Clean(currRing);
        omFree(G1.labels);
    }
    
    //std::cout << "[DEBUG] Assigning new labels" << std::endl;
    // Create a copy of eliminatedVars for labels
    lists labelsCopy = (lists)omAlloc(sizeof(sleftv));
    labelsCopy->Init(IDELEMS(I_std));
    for (int i = 0; i < IDELEMS(I_std); i++) {
        if (eliminatedVars->m[i].rtyp == POLY_CMD) {
            labelsCopy->m[i].rtyp = POLY_CMD;
            labelsCopy->m[i].data = (void*)p_Copy((poly)eliminatedVars->m[i].data, currRing);
        }
    }
    
    G1.labels = labelsCopy;
    G1.elimvars = eliminatedVars; // Store eliminated variables for later use
    
    //std::cout << "[DEBUG] Final cleanup" << std::endl;
    // Clean up
    id_Delete(&I, currRing);
    id_Delete(&I_std, currRing);
    //std::cout << "[DEBUG] Exiting eliminateVariables" << std::endl;
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

    for (int i = 0; i < getSize(G.edges); i++) {
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



// Compute Baikov matrix for a labeled graph
LabeledGraph computeBaikovMatrix(const LabeledGraph& G) {
    std::cout << "[DEBUG] Entering computeBaikovMatrix" << std::endl;
    
    // Create a copy of the input graph
    LabeledGraph G1;
    G1.vertices = G.vertices;
    G1.edges = G.edges;
    G1.over = G.over;
    G1.labels = G.labels;
    G1.overpoly = G.overpoly;
    G1.elimvars = G.elimvars;
    
    // Set the current ring to G.over
    ring oldRing = currRing;
    if (G.over != NULL) {
        std::cout << "[DEBUG] Switching to graph ring" << std::endl;
        rChangeCurrRing(G.over);
    }
    
    // Create a new ring for the Baikov matrix
    std::cout << "[DEBUG] Creating Baikov matrix ring" << std::endl;
    G1.baikovover = rCopy(currRing);
    if (G1.baikovover == NULL) {
        std::cout << "[ERROR] Failed to create Baikov ring" << std::endl;
        return G1;
    }
    
    rChangeCurrRing(G1.baikovover);
    
    // Initialize the Baikov matrix
    int n = getSize(G.edges);
    std::cout << "[DEBUG] Creating " << n << "x" << n << " Baikov matrix" << std::endl;
    G1.baikovmatrix = mpNew(n, n);
    
    // Compute the Baikov matrix entries
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (G.edges->m[i].rtyp != LIST_CMD || G.edges->m[j].rtyp != LIST_CMD) {
                std::cout << "[ERROR] Invalid edge type at " << i << "," << j << std::endl;
                continue;
            }
            
            lists edge1 = (lists)G.edges->m[i].Data();
            lists edge2 = (lists)G.edges->m[j].Data();
            poly p = NULL;
            
            if (edge1 == NULL || edge2 == NULL || edge1->nr < 1 || edge2->nr < 1) {
                std::cout << "[ERROR] Invalid edge data at " << i << "," << j << std::endl;
                continue;
            }
            
            // Get vertices of both edges
            if (edge1->m[0].rtyp != INT_CMD || edge1->m[1].rtyp != INT_CMD ||
                edge2->m[0].rtyp != INT_CMD || edge2->m[1].rtyp != INT_CMD) {
                std::cout << "[ERROR] Invalid vertex data at " << i << "," << j << std::endl;
                continue;
            }
            
            int v1 = (int)(long)edge1->m[0].Data();
            int v2 = (int)(long)edge1->m[1].Data();
            int w1 = (int)(long)edge2->m[0].Data();
            int w2 = (int)(long)edge2->m[1].Data();
            
            // Check if edges share vertices
            if ((v1 == w1 && v2 == w2) || (v1 == w2 && v2 == w1)) {
                p = p_ISet(1, currRing); // Set diagonal elements to 1
               //std::cout << "[DEBUG] Setting diagonal element (" << i << "," << j << ") to 1" << std::endl;
            } else if (v1 == w1 || v1 == w2 || v2 == w1 || v2 == w2) {
                // Edges share a vertex - set appropriate matrix element
                if (G.labels != NULL && i <= G.labels->nr && j <= G.labels->nr &&
                    G.labels->m[i].rtyp == POLY_CMD && G.labels->m[j].rtyp == POLY_CMD) {
                    
                    poly label1 = (poly)G.labels->m[i].Data();
                    poly label2 = (poly)G.labels->m[j].Data();
                    if (label1 && label2) {
                        p = pp_Mult_qq(p_Copy(label1, currRing), p_Copy(label2, currRing), currRing);
                       // std::cout << "[DEBUG] Setting shared vertex element (" << i << "," << j << ") to " 
                                  //<< p_String(p, currRing) << std::endl;
                    }
                }
            }
            
            MATELEM(G1.baikovmatrix, i + 1, j + 1) = p;
            if (i != j && p != NULL) {
                // Make matrix symmetric
                MATELEM(G1.baikovmatrix, j + 1, i + 1) = p_Copy(p, currRing);
            }
        }
    }
    
    // Switch back to the original ring
    if (oldRing != NULL) {
        std::cout << "[DEBUG] Switching back to original ring" << std::endl;
        rChangeCurrRing(oldRing);
    }
    
    std::cout << "[DEBUG] Exiting computeBaikovMatrix" << std::endl;
    return G1;
}

// Compute M1 ideal for IBP relations
ideal computeM1(const LabeledGraph& G) {
    std::cout << "[DEBUG] Entering computeM1" << std::endl;
    
    // Check if Baikov ring and matrix are initialized
    if (!G.baikovover) {
        std::cout << "[ERROR] Baikov ring not initialized" << std::endl;
        return NULL;
    }
    if (!G.baikovmatrix) {
        std::cout << "[ERROR] Baikov matrix not initialized" << std::endl;
        return NULL;
    }
    
    // Ensure we are in the Baikov ring
    if (currRing != G.baikovover) {
        std::cout << "[ERROR] Not in Baikov ring" << std::endl;
        return NULL;
    }
    
    std::cout << "[DEBUG] Getting Baikov matrix" << std::endl;
    matrix B = G.baikovmatrix;
    if (!B) {
        std::cout << "[ERROR] Baikov matrix is null" << std::endl;
        return NULL;
    }
    
    // Get ring dimensions
    std::cout << "[DEBUG] Getting ring dimensions" << std::endl;
    int n = rVar(currRing);       // Total number of variables
    int m = rPar(G.over);         // Number of parameters
    int L = rVar(G.over);         // Number of loop variables
    int E = n - L;                // Number of edge variables
    
    // Print matrix B for debugging
    std::cout << "[DEBUG] Baikov matrix contents:" << std::endl;
    for (int i = 1; i <= MATROWS(B); i++) {
        for (int j = 1; j <= MATCOLS(B); j++) {
            poly p = MATELEM(B, i, j);
            std::cout << "  B[" << i << "," << j << "] = " << (p ? p_String(p, currRing) : "NULL") << std::endl;
        }
    }
    
    std::cout << "[DEBUG] Matrix dimensions:" << std::endl;
    std::cout << "  n (vars in currRing) = " << n << std::endl;
    std::cout << "  m (params in G.over) = " << m << std::endl;
    std::cout << "  E = " << E << std::endl;
    std::cout << "  L (vars in G.over) = " << L << std::endl;
    
    // Validate dimensions
    if (n <= 0 || E <= 0 || L <= 0) {
        std::cout << "[ERROR] Invalid matrix dimensions" << std::endl;
        return NULL;
    }

    // Create derivative matrix C
    int rows = E * L + L * (L + 1) / 2;
    std::cout << "[DEBUG] Creating derivative matrix " << rows << "x" << n << std::endl;
    matrix C = mpNew(rows, n);
    if (!C) {
        std::cout << "[ERROR] Failed to create derivative matrix" << std::endl;
        return NULL;
    }
    int tem = 0;

    // Initialize matrix elements to zero
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= n; j++) {
            MATELEM(C, i, j) = NULL;
        }
    }
    
    // Compute derivatives for E*L part
    std::cout << "[DEBUG] Computing E*L derivatives" << std::endl;
    for (int i = 1; i <= E; i++) {
        for (int j = E + 1; j <= E + L; j++) {
            tem++;
            std::cout << "  Processing B[" << i << "," << j << "]" << std::endl;
            poly bij = MATELEM(B, i, j);
            if (!bij) {
                std::cout << "  Matrix element is null" << std::endl;
                continue;
            }
            for (int k = 1; k <= n; k++) {
                std::cout << "    Computing derivative w.r.t var " << k << std::endl;
                poly derivative = pDiff(bij, k);
                MATELEM(C, tem, k) = derivative;
            }
        }
    }
    
    // Compute derivatives for L*(L+1)/2 part
    std::cout << "[DEBUG] Computing L*(L+1)/2 derivatives" << std::endl;
    for (int i = E + 1; i <= E + L; i++) {
        for (int j = i; j <= E + L; j++) {
            tem++;
            std::cout << "  Processing B[" << i << "," << j << "]" << std::endl;
            poly bij = MATELEM(B, i, j);
            if (!bij) {
                std::cout << "  Matrix element is null" << std::endl;
                continue;
            }
            for (int k = 1; k <= n; k++) {
                std::cout << "    Computing derivative w.r.t var " << k << std::endl;
                poly derivative = pDiff(bij, k);
                MATELEM(C, tem, k) = derivative;
            }
        }
    }
    
    // Create M1 as an ideal
    ideal M1 = idInit(rows, 1); // One polynomial per row
    for (int i = 0; i < MATROWS(C); i++) {
        poly rowPoly = NULL;
        for (int j = 0; j < MATCOLS(C); j++) {
            poly term = MATELEM(C, i + 1, j + 1);
            if (term) {
                rowPoly = p_Add_q(rowPoly, p_Copy(term, currRing), currRing);
            }
        }
        M1->m[i] = rowPoly;
    }
    // Note: Do not set M1->rank, as it’s an ideal (implicitly rank 1)

    // Clean up matrix C
    if (C != NULL) {
        mp_Delete(&C, currRing);
    }
    
    std::cout << "[DEBUG] Exiting computeM1" << std::endl;
    return M1;
}
// Compute M2 module for IBP relations
ideal computeM2(const LabeledGraph& G, lists Nu) {
    std::cout << "[DEBUG] Entering computeM2" << std::endl;
    
    // Save current ring and switch to Baikov ring
    if (!G.baikovover) {
        std::cout << "[ERROR] Baikov ring not initialized" << std::endl;
        return NULL;
    }
    if (!G.baikovmatrix) {
        std::cout << "[ERROR] Baikov matrix not initialized" << std::endl;
        return NULL;
    }
    
    // We expect to already be in the Baikov ring when this function is called
    if (currRing != G.baikovover) {
        std::cout << "[ERROR] Not in Baikov ring" << std::endl;
        return NULL;
    }
    if (!G.baikovmatrix) {
        std::cout << "[ERROR] Baikov matrix not initialized" << std::endl;
        return NULL;
    }
    if (!Nu) {
        std::cout << "[ERROR] Nu list is null" << std::endl;
        return NULL;
    }
    if (Nu->nr < 0) {
        std::cout << "[ERROR] Nu list is empty" << std::endl;
        return NULL;
    }
    
    // Switch to Baikov ring
    std::cout << "[DEBUG] Switching to Baikov ring" << std::endl;
    rChangeCurrRing(G.baikovover);
    
    // Print ring info
    std::cout << "[DEBUG] Current ring info:" << std::endl;
    std::cout << "  Variables: " << rVar(currRing) << std::endl;
    std::cout << "  Parameters: " << rPar(currRing) << std::endl;
    std::cout << "  Characteristic: " << rChar(currRing) << std::endl;
    
    // Print Nu list contents
    std::cout << "[DEBUG] Nu list contents:" << std::endl;
    for (int i = 0; i <= Nu->nr; i++) {
        if (Nu->m[i].rtyp != INT_CMD) {
            std::cout << "[ERROR] Nu element " << i << " is not an integer" << std::endl;
            return NULL;
        }
        int val = (int)(long)Nu->m[i].Data();
        std::cout << "  Nu[" << i << "] = " << val << std::endl;
    }
    
    int n = rVar(currRing);
    if (Nu->nr + 1 != n) {
        std::cout << "[ERROR] Nu list size (" << Nu->nr + 1 << ") does not match number of variables (" << n << ")" << std::endl;
        return NULL;
    }
    
    // Create M2 module
    ideal M2 = idInit(1, 1);
    M2->rank = 1;  // Set rank to indicate it's a module
    int pos = 0;
    
    for (int i = 0; i <= Nu->nr; i++) {
        if (Nu->m[i].rtyp != INT_CMD) {
            std::cout << "[ERROR] Invalid Nu entry type at position " << i << std::endl;
            continue;
        }
        
        int nu_i = (int)(long)Nu->m[i].Data();
        if (nu_i > 0) {
            for (int j = 1; j <= nu_i; j++) {
                poly p = p_ISet(1, currRing);
                p_SetExp(p, i+1, j, currRing);
                p_Setm(p, currRing);
                
                M2->m[pos] = p_Copy(p, currRing);
                pos++;
            }
        }
    }
    
    // Resize M2 to actual size
    ideal M2_resized = idInit(pos, 1);
    for (int i = 0; i < pos; i++) {
        M2_resized->m[i] = M2->m[i];
    }
    M2_resized->rank = 1;  // Set rank to indicate it's a module
    
    // Clean up original M2
    id_Delete(&M2, currRing);
    
    std::cout << "[DEBUG] Exiting computeM2" << std::endl;
    return M2_resized;
}

// Compute many IBP relations
SetIBP computeManyIBP(const LabeledGraph& G, lists setNu) {
    std::cout << "[DEBUG] Entering computeManyIBP" << std::endl;
    
    SetIBP result;
    std::cout << "[DEBUG] Initializing result" << std::endl;
    result.over = G.over;
    result.IBP = NULL;
    
    // Save current ring and switch to Baikov ring
    std::cout << "[DEBUG] Saving current ring" << std::endl;
    ring oldRing = currRing;
    std::cout << "[DEBUG] Switching to Baikov ring" << std::endl;
    if (!G.baikovover) {
        std::cout << "[ERROR] Baikov ring is null" << std::endl;
        return result;
    }
    if (G.baikovmatrix == NULL) {
        std::cout << "[ERROR] Baikov matrix is null" << std::endl;
        return result;
    }
    rChangeCurrRing(G.baikovover);
    std::cout << "[DEBUG] Ring switched successfully" << std::endl;
    
    // Print ring info
    std::cout << "[DEBUG] Current ring info:" << std::endl;
    std::cout << "  Variables: " << rVar(currRing) << std::endl;
    std::cout << "  Parameters: " << rPar(currRing) << std::endl;
    std::cout << "  Characteristic: " << rChar(currRing) << std::endl;
    
    // Check if setNu is valid
    if (setNu == NULL || setNu->nr < 0) {
        std::cout << "[ERROR] Invalid setNu" << std::endl;
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    
    // Get first Nu from setNu
    std::cout << "[DEBUG] Getting Nu from setNu" << std::endl;
    if (setNu->m[0].rtyp != LIST_CMD) {
        std::cout << "[ERROR] First element of setNu is not a list" << std::endl;
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    lists Nu = (lists)setNu->m[0].Data();
    if (Nu == NULL) {
        std::cout << "[ERROR] Invalid Nu in setNu" << std::endl;
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    std::cout << "[DEBUG] Nu list has " << Nu->nr + 1 << " elements" << std::endl;
    
    // Validate Nu list
    if (Nu->nr < 0) {
        std::cout << "[ERROR] Nu list is empty" << std::endl;
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    for (int i = 0; i <= Nu->nr; i++) {
        if (Nu->m[i].rtyp != INT_CMD) {
            std::cout << "[ERROR] Nu element " << i << " is not an integer" << std::endl;
            if (oldRing != NULL) {
                rChangeCurrRing(oldRing);
            }
            return result;
        }
        int val = (int)(long)Nu->m[i].Data();
        std::cout << "[DEBUG] Nu[" << i << "] = " << val << std::endl;
    }
    
    // Compute M1 and M2
    std::cout << "[DEBUG] Computing M1" << std::endl;
    ideal M1 = computeM1(G);
    if (!M1) {
        std::cout << "[ERROR] Failed to compute M1" << std::endl;
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    std::cout << "[DEBUG] M1 size: " << IDELEMS(M1) << std::endl;
    
    // Print M1 contents
    std::cout << "[DEBUG] M1 contents:" << std::endl;
    for (int i = 0; i < IDELEMS(M1); i++) {
        poly p = M1->m[i];
        std::cout << "  M1[" << i << "] = " << (p ? p_String(p, currRing) : "NULL") << std::endl;
    }
    
    std::cout << "[DEBUG] Computing M2" << std::endl;
    ideal M2 = computeM2(G, Nu);
    if (!M2) {
        std::cout << "[ERROR] Failed to compute M2" << std::endl;
        id_Delete(&M1, currRing);
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    std::cout << "[DEBUG] M2 size: " << IDELEMS(M2) << std::endl;
    
    // Print M2 contents
    std::cout << "[DEBUG] M2 contents:" << std::endl;
    for (int i = 0; i < IDELEMS(M2); i++) {
        poly p = M2->m[i];
        std::cout << "  M2[" << i << "] = " << (p ? p_String(p, currRing) : "NULL") << std::endl;
    }
    
    // Compute intersection
    std::cout << "[DEBUG] Computing intersection" << std::endl;
    ideal MM = idSect(M1, M2);
    if (!MM) {
        std::cout << "[ERROR] Failed to compute intersection" << std::endl;
        id_Delete(&M1, currRing);
        id_Delete(&M2, currRing);
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    std::cout << "[DEBUG] Intersection size: " << IDELEMS(MM) << std::endl;
    
    // Print intersection contents
    std::cout << "[DEBUG] Intersection contents:" << std::endl;
    for (int i = 0; i < IDELEMS(MM); i++) {
        poly p = MM->m[i];
        std::cout << "  MM[" << i << "] = " << (p ? p_String(p, currRing) : "NULL") << std::endl;
    }
    
    // Clean up M1 and M2 since we don't need them anymore
    id_Delete(&M1, currRing);
    id_Delete(&M2, currRing);
    
    // Compute standard basis
    std::cout << "[DEBUG] Computing standard basis" << std::endl;
    ideal M = kStd(MM, currRing->qideal, testHomog, NULL);
    if (!M) {
        std::cout << "[ERROR] Failed to compute standard basis" << std::endl;
        id_Delete(&MM, currRing);
        if (oldRing != NULL) {
            rChangeCurrRing(oldRing);
        }
        return result;
    }
    std::cout << "[DEBUG] Standard basis size: " << IDELEMS(M) << std::endl;
    
    // Print standard basis contents
    std::cout << "[DEBUG] Standard basis contents:" << std::endl;
    for (int i = 0; i < IDELEMS(M); i++) {
        poly p = M->m[i];
        std::cout << "  M[" << i << "] = " << (p ? p_String(p, currRing) : "NULL") << std::endl;
    }
    
    // Clean up MM since we don't need it anymore
    id_Delete(&MM, currRing);
    
    // Convert module to list
    lists L = (lists)omAlloc0(sizeof(slists));
    L->Init(IDELEMS(M));
    for (int i = 0; i < IDELEMS(M); i++) {
        L->m[i].rtyp = POLY_CMD;
        L->m[i].data = (void*)p_Copy(M->m[i], currRing);
    }
    result.IBP = L;
    
    // Switch back to original ring
    if (oldRing != NULL) {
        rChangeCurrRing(oldRing);
    }
    
    return result;
}