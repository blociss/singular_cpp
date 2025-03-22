#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>

// Struct definitions mimicking Singular's newstruct
struct Graph {
    lists vertices;
    lists edges;
};

struct Net {
    lists rows;
};

struct LabeledGraph {
    lists vertices;
    lists edges;
    ring over;
    lists labels;
};

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
    std::cout << "[DEBUG] netList: listSize=" << listSize << "\n";

    for (int j = 0; j < listSize; j++) {
        if (L1->m[j].rtyp != LIST_CMD) {
            std::cerr << "Expected list type at index " << j << "\n";
            continue;
        }
        std::cout << "[DEBUG] netList: Processing element " << j << "\n";
        lists subList = (lists)L1->m[j].Data();
        if (!subList) {
            std::cerr << "Null sublist at index " << j << "\n";
            continue;
        }
        int subSize = subList->nr + 1;
        std::cout << "[DEBUG] netList: subSize=" << subSize << "\n";

        std::string subResult = "[";
        for (int k = 0; k < subSize; k++) {
            std::cout << "[DEBUG] netList: Processing sub-element " << k << "\n";
            if (subList->m[k].rtyp != INT_CMD) {
                std::cerr << "Expected integer type at subindex " << k << "\n";
                continue;
            }
            long intValue = (long)subList->m[k].Data();
            std::cout << "[DEBUG] netList: value=" << intValue << "\n";
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
void printNet(Net N) {
    std::cout << "[DEBUG] Entering printNet\n";
    lists L = N.rows;
    int listSize = L ? L->nr + 1 : 0;
    std::cout << "[DEBUG] printNet: listSize=" << listSize << "\n";
    for (int j = 0; j < listSize; j++) {
        std::cout << "[DEBUG] printNet: Printing row " << j << "\n";
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

// Eliminate variables from a labeled graph using balancing equations
LabeledGraph eliminateVariables(LabeledGraph G) {
    ring R = currRing;
    ideal I = balancingIdeal(G);

    // Create ideal with single polynomial
    ideal Q = idInit(1, 1); // Empty ideal for quotient
    tHomog h = testHomog;
    ideal I_std = kStd(I, Q, h, nullptr);

    LabeledGraph G1 = G;
    lists eliminatedVars = (lists)omAlloc(sizeof(sleftv));
    eliminatedVars->Init(IDELEMS(I_std));

    for (int i = 0; i < IDELEMS(I_std); i++) {
        poly ld = p_Head(I_std->m[i], currRing);
        poly ta = p_Sub(p_Copy(I_std->m[i], currRing), p_Copy(ld, currRing), currRing);

        ideal tmpI = idInit(1, 1);
        tmpI->m[0] = p_Copy(ld, currRing);
        poly ld_orig = p_Copy(tmpI->m[0], currRing);

        tmpI->m[0] = p_Copy(ta, currRing);
        poly ta_orig = p_Copy(tmpI->m[0], currRing);

        eliminatedVars->m[i].rtyp = POLY_CMD;
        eliminatedVars->m[i].data = (void*)ld_orig;

        G1 = substituteGraph(G1, ld_orig, ta_orig);

        p_Delete(&ld, currRing);
        p_Delete(&ta, currRing);
        id_Delete(&tmpI, currRing);
    }

    G1.labels = eliminatedVars;
    id_Delete(&I, currRing);
    id_Delete(&I_std, currRing);
    return G1;
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


int main() {
    std::cout << "[DEBUG] Entering main\n";
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    std::cout << "[DEBUG] Singular initialized\n";

    // Create ring with variables q(1)..q(6)
    char* varNames[] = {(char*)"q(1)", (char*)"q(2)", (char*)"q(3)", (char*)"q(4)", (char*)"q(5)", (char*)"q(6)"};
    ring r = rDefault(0, 6, varNames);
    rComplete(r);
    rChangeCurrRing(r);
    std::cout << "[DEBUG] Ring created\n";

    // Create vertices list
    int verticesData[] = {1, 2, 3, 4};
    lists vertices = createIntList(verticesData, 4);

    // Create edges list with pairs
    int edgesData[][2] = {{1, 3}, {1, 2}, {1, 2}, {2, 4}, {3, 4}, {3, 4}};
    lists edges = createEdgeList(edgesData, 6, nullptr, 0);

    // Create labels list using ring variables
    lists labels = (lists)omAlloc(sizeof(sleftv));
    labels->Init(6);
    for (int i = 0; i < 6; i++) {
        poly p = p_ISet(1, r);  // Create polynomial 1
        p_SetExp(p, i + 1, 1, r);  // Set exponent for q(i+1) to 1
        p_Setm(p, r);  // Update leading monomial
        labels->m[i].rtyp = POLY_CMD;
        labels->m[i].data = (void*)p;
    }
    labels->nr = 5;

    // Create labeled graph
    LabeledGraph G = makeLabeledGraph(vertices, edges, r, labels);
    std::cout << "[DEBUG] Labeled graph created\n";

    std::cout << "EXAMPLE:" << std::endl;
    printLabeledGraph(G);

    // Cleanup
    vertices->Clean(r);
    omFree(vertices);
    edges->Clean(r);
    omFree(edges);
    labels->Clean(r);
    omFree(labels);
    rKill(r);
    std::cout << "[DEBUG] Exiting main\n";
    return 0;
}