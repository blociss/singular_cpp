#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "feynman.h"



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
    std::cout << "[DEBUG] Eliminating variables\n";
    G = eliminateVariables(G);
    removeElimVars(G);
    std::cout << "[DEBUG] Eliminated variables: " << G.elimvars << std::endl;
    
    // Compute and print Baikov matrix
    std::cout << "\n[DEBUG] Computing Baikov matrix...\n";
    LabeledGraph G_baikov = computeBaikovMatrix(G);
    std::cout << "[DEBUG] Baikov matrix computed\n";
    
    // Print the Baikov matrix
    if (G_baikov.baikovmatrix != NULL) {
        std::cout << "Baikov matrix:\n";
        for (int i = 0; i < MATROWS(G_baikov.baikovmatrix); i++) {
            for (int j = 0; j < MATCOLS(G_baikov.baikovmatrix); j++) {
                poly p = MATELEM(G_baikov.baikovmatrix, i + 1, j + 1);
                if (p != NULL) {
                    std::cout << "[" << i << "," << j << "] = " << p_String(p, G_baikov.baikovover) << "\n";
                }
            }
        }
        std::cout << std::endl;
    }
    
    ideal I = balancingIdeal(G);  

std::cout << "[DEBUG] Balancing ideal computed\n";
    for (int i = 0; i < IDELEMS(I); i++) {
        std::cout<<"[DEBUG] I->m["<<i<<"]="<<pString((poly)I->m[i])<<std::endl;
    }
    
    // Example of IBP computation
    std::cout << "\n[DEBUG] Computing IBP relations...\n";
    
    // First compute M1 directly
    std::cout << "[DEBUG] Computing M1 directly...\n";
    ring savedRing = currRing;
    rChangeCurrRing(G_baikov.baikovover);
    ideal M1_direct = computeM1(G_baikov);
    if (M1_direct != NULL) {
        std::cout << "[DEBUG] M1 computed directly, size: " << IDELEMS(M1_direct) << std::endl;
        for (int i = 0; i < IDELEMS(M1_direct); i++) {
            std::cout << "  M1[" << i << "] = " << p_String(M1_direct->m[i], currRing) << std::endl;
        }
    } else {
        std::cout << "[ERROR] Failed to compute M1 directly" << std::endl;
    }
    rChangeCurrRing(savedRing);
    
    // Create Nu list for testing M2
    std::cout << "[DEBUG] Creating Nu list for M2...\n";
    lists Nu = (lists)omAlloc(sizeof(sleftv));
    ring oldRing = currRing;
    rChangeCurrRing(G_baikov.baikovover);
    int n = rVar(currRing);
    rChangeCurrRing(oldRing);
    Nu->Init(n);
    for (int i = 0; i < n; i++) {
        Nu->m[i].rtyp = INT_CMD;
        Nu->m[i].data = (void*)(long)(1);  // Set all Nu elements to 1 for simplicity
    }
    Nu->nr = n - 1;
    
    // Compute M2 directly
    std::cout << "[DEBUG] Computing M2 directly...\n";
    rChangeCurrRing(G_baikov.baikovover);
    ideal M2_direct = computeM2(G_baikov, Nu);
    if (M2_direct != NULL) {
        std::cout << "[DEBUG] M2 computed directly, size: " << IDELEMS(M2_direct) << std::endl;
        for (int i = 0; i < IDELEMS(M2_direct); i++) {
            std::cout << "  M2[" << i << "] = " << p_String(M2_direct->m[i], currRing) << std::endl;
        }
    } else {
        std::cout << "[ERROR] Failed to compute M2 directly" << std::endl;
    }
    rChangeCurrRing(savedRing);
    
    // Create setNu list containing Nu for computeManyIBP
    std::cout << "[DEBUG] Creating setNu for computeManyIBP...\n";
    lists setNu = (lists)omAlloc(sizeof(sleftv));
    setNu->Init(1);
    setNu->m[0].rtyp = LIST_CMD;
    setNu->m[0].data = (void*)Nu;
    setNu->nr = 0;
    
    // Now try computeManyIBP
    std::cout << "[DEBUG] Computing many IBP relations...\n";
    ideal M1 = computeM1(G_baikov);
    if (M1 != NULL) {
        std::cout << "M1 computed successfully\n";
        std::cout << "M1 size: " << IDELEMS(M1) << ", rank: " << M1->rank << std::endl;
    }
    
    // Compute M2
    std::cout << "Computing M2...\n";
    ideal M2 = computeM2(G_baikov, Nu);
    if (M2 != NULL) {
        std::cout << "M2 computed successfully\n";
        std::cout << "M2 size: " << IDELEMS(M2) << ", rank: " << M2->rank << std::endl;
    }
    
    // Compute many IBP relations
    std::cout << "Computing many IBP relations...\n";
    SetIBP ibp = computeManyIBP(G_baikov, setNu);
    if (ibp.IBP != NULL) {
        std::cout << "IBP relations computed successfully\n";
        std::cout << "Number of IBP relations: " << ibp.IBP->nr + 1 << std::endl;
    }
    // Cleanup in reverse order of creation
    std::cout << "[DEBUG] Starting cleanup\n";
    
    // First cleanup IBP-related objects
    if (M1_direct != NULL) {
        std::cout << "[DEBUG] Cleaning up M1_direct module\n";
        id_Delete((ideal*)&M1_direct, G_baikov.baikovover);
    }
    
    if (M2_direct != NULL) {
        std::cout << "[DEBUG] Cleaning up M2_direct module\n";
        id_Delete((ideal*)&M2_direct, G_baikov.baikovover);
    }
    
    if (M1 != NULL) {
        std::cout << "[DEBUG] Cleaning up M1 module\n";
        id_Delete((ideal*)&M1, currRing);
    }
    
    if (M2 != NULL) {
        std::cout << "[DEBUG] Cleaning up M2 module\n";
        id_Delete((ideal*)&M2, currRing);
    }
    
    if (ibp.IBP != NULL) {
        std::cout << "[DEBUG] Cleaning up IBP list\n";
        ibp.IBP->Clean();
    }
    
    if (Nu != NULL) {
        std::cout << "[DEBUG] Cleaning up Nu list\n";
        Nu->Clean();
        omFree(Nu);
    }
    
    if (setNu != NULL) {
        std::cout << "[DEBUG] Cleaning up setNu list\n";
        setNu->Clean();
        omFree(setNu);
    }
    
    // Then cleanup the Baikov matrix and balancing ideal
    ring cleanupRing = currRing;
    
    if (G_baikov.baikovmatrix != NULL) {
        std::cout << "[DEBUG] Cleaning up Baikov matrix\n";
        if (G_baikov.baikovover != NULL) {
            rChangeCurrRing(G_baikov.baikovover);
            id_Delete((ideal*)&G_baikov.baikovmatrix, G_baikov.baikovover);
            rChangeCurrRing(cleanupRing);
        }
    }
    
    if (G_baikov.baikovover != NULL) {
        std::cout << "[DEBUG] Cleaning up Baikov ring\n";
        if (G_baikov.baikovover != cleanupRing) {
            rKill(G_baikov.baikovover);
        }
    }
    
    if (I != NULL) {
        std::cout << "[DEBUG] Cleaning up balancing ideal\n";
        id_Delete(&I, currRing);
    }
    
    // Then cleanup the graph's memory
    std::cout << "[DEBUG] Cleaning up graph memory\n";
    if (G.labels != NULL) {
        std::cout << "[DEBUG] Cleaning up graph labels\n";
        G.labels->Clean(currRing);
        omFree(G.labels);
        G.labels = NULL;
    }
    if (G.elimvars != NULL) {
        std::cout << "[DEBUG] Cleaning up eliminated variables\n";
        G.elimvars->Clean(currRing);
        omFree(G.elimvars);
        G.elimvars = NULL;
    }
    
    // Finally cleanup the input data
    std::cout << "[DEBUG] Cleaning up input data\n";
    if (vertices != NULL) {
        vertices->Clean(r);
        omFree(vertices);
        vertices = NULL;
    }
    if (edges != NULL) {
        edges->Clean(r);
        omFree(edges);
        edges = NULL;
    }
    if (labels != NULL) {
        labels->Clean(r);
        omFree(labels);
        labels = NULL;
    }
    
    // Kill the rings in reverse order
    std::cout << "[DEBUG] Killing rings\n";
    if (currRing != r) {
        rKill(currRing);
    }
    rKill(r);
    
    std::cout << "[DEBUG] Cleanup complete\n";
    std::cout << "[DEBUG] Exiting main\n";
    return 0;
}