#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "polys/ext_fields/transext.h"

ring createRing(char **extNames, int extCount, char **varNames, int varCount)
{
    std::cout << "[DEBUG] Creating ring with " << varCount << " variables" << std::endl;
    
    // Create the coefficient field Q[t(1), t(2), D]
    ring extRing = rDefault(0, extCount, extNames);
    if (!extRing) {
        std::cout << "[ERROR] Failed to create extension ring" << std::endl;
        return NULL;
    }
    
    TransExtInfo extParam;
    extParam.r = extRing;
    coeffs cf = nInitChar(n_transExt, &extParam);
    if (!cf) {
        std::cout << "[ERROR] Failed to create coefficient field" << std::endl;
        rKill(extRing);
        return NULL;
    }

    // Define variable names for the ring (e.g., z(1) to z(9))
    char **vars = (char **)omAlloc0(varCount * sizeof(char*));
    if (!vars) {
        std::cout << "[ERROR] Failed to allocate vars array" << std::endl;
        nKillChar(cf);
        rKill(extRing);
        return NULL;
    }
    
    for (int i = 0; i < varCount; ++i)
    {
        vars[i] = (char *)omAlloc(10 * sizeof(char));
        if (!vars[i]) {
            std::cout << "[ERROR] Failed to allocate var[" << i << "]" << std::endl;
            // Clean up previously allocated vars
            for (int j = 0; j < i; ++j) {
                omFree(vars[j]);
            }
            omFree(vars);
            nKillChar(cf);
            rKill(extRing);
            return NULL;
        }
        sprintf(vars[i], "%s(%d)", varNames[0], i + 1);
    }

    // Define the orders: dp for variables, C for coefficients
    const int numBlocks = 2; // One for dp, one for C
    rRingOrder_t *order = (rRingOrder_t *)omAlloc0(numBlocks * sizeof(rRingOrder_t));
    if (!order) {
        std::cout << "[ERROR] Failed to allocate order array" << std::endl;
        for (int i = 0; i < varCount; ++i) {
            omFree(vars[i]);
        }
        omFree(vars);
        nKillChar(cf);
        rKill(extRing);
        return NULL;
    }
    
    order[0] = ringorder_dp;    // Degree reverse lexicographical ordering for variables
    order[1] = ringorder_C;     // Ordering for coefficients

    // Define the blocks
    int *block0 = (int *)omAlloc0(2 * sizeof(int));
    int *block1 = (int *)omAlloc0(2 * sizeof(int));
    
    if (!block0 || !block1) {
        std::cout << "[ERROR] Failed to allocate block arrays" << std::endl;
        if (block0) omFree(block0);
        if (block1) omFree(block1);
        omFree(order);
        for (int i = 0; i < varCount; ++i) {
            omFree(vars[i]);
        }
        omFree(vars);
        nKillChar(cf);
        rKill(extRing);
        return NULL;
    }
    
    // Block 0: dp ordering for variables
    block0[0] = 1;
    block0[1] = varCount;
    
    // Block 1: C ordering for coefficients
    block1[0] = 0;
    block1[1] = 0;

    // Create the ring with 2 ordering blocks
    std::cout << "[DEBUG] Creating ring with " << varCount << " variables and " << numBlocks << " blocks" << std::endl;
    ring R = rDefault(cf, varCount, vars, numBlocks, order, block0, block1);
    if (!R) {
        std::cout << "[ERROR] Failed to create ring" << std::endl;
        omFree(block0);
        omFree(block1);
        omFree(order);
        for (int i = 0; i < varCount; ++i) {
            omFree(vars[i]);
        }
        omFree(vars);
        nKillChar(cf);
        rKill(extRing);
        return NULL;
    }

    // Clean up temporary allocations
    omFree(block0);
    omFree(block1);
    omFree(order);
    for (int i = 0; i < varCount; ++i) {
        omFree(vars[i]);
    }
    omFree(vars);
    
    // Note: We don't free cf or extRing here as they are now owned by R
    std::cout << "[DEBUG] Ring created successfully" << std::endl;
    return R;
}




int main()
{
    std::cout << "[DEBUG] Entering main\n";
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
    std::cout << "[DEBUG] Singular initialized\n";

   
    // Create ring with variables q(1)..q(6)
    char* varNames[] = {(char*)"q(1)", (char*)"q(2)", (char*)"q(3)", (char*)"q(4)", (char*)"q(5)", (char*)"q(6)"};
    ring r = rDefault(0, 6, varNames);
    rComplete(r);
    rChangeCurrRing(r);
    std::cout << "[DEBUG] Ring created\n";

    // Create vertices list with 6 vertices
    int verticesData[] = {1, 2, 3, 4, 5, 6};
    lists vertices = createIntList(verticesData, 6);

    // Create edges list with 7 bounded edges and 4 unbounded edges
    int boundedEdges[][2] = {{6,1}, {4,6}, {1,2}, {3,5}, {4,3}, {2,5}, {5,6}};
    int unboundedEdges[] = {1, 2, 3, 4};
    lists edges = createEdgeList(boundedEdges, 7, unboundedEdges, 4);

    // Create labels list using ring variables z(1) to z(11) initially
    lists labels = (lists)omAlloc(sizeof(sleftv));
    labels->Init(11); // 11 edges initially
    for (int i = 0; i < 11; i++)
    {
        poly p = p_ISet(1, r);       // Create polynomial 1
        p_SetExp(p, (i < 9 ? i + 1 : 1), 1, r); // Use z(1) for extras after z(9)
        p_Setm(p, r);                // Update leading monomial
        labels->m[i].rtyp = POLY_CMD;
        labels->m[i].data = (void*)p;
    }
    labels->nr = 10;

    // Create labeled graph
    LabeledGraph G = makeLabeledGraph(vertices, edges, r, labels);
    std::cout << "[DEBUG] Labeled graph created\n";

    std::cout << "EXAMPLE:" << std::endl;
    printLabeledGraph(G);

    // Test Graph case
    std::cout << "\n=== Testing Graph Input ===" << std::endl;
    Graph G1 = makeGraph(vertices, edges);
    std::cout << "Input Graph G1:" << std::endl;
    printGraph(G1);
    
    std::cout << "\n[DEBUG] Computing Baikov matrix from Graph...\n";
    LabeledGraph result1 = computeBaikovMatrix(G1);
    std::cout << "\nBaikov Matrix Result from Graph:" << std::endl;
    printLabeledGraph(result1);
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    std::cout << "[DEBUG] Baikov ring: " << rString(result1.baikovover) << std::endl;
    
    // Test LabeledGraph case
    std::cout << "\n=== Testing LabeledGraph Input ===" << std::endl;
    std::cout << "Input LabeledGraph G:" << std::endl;
    printLabeledGraph(G);
    
    std::cout << "\n[DEBUG] Computing Baikov matrix from LabeledGraph...\n";
    LabeledGraph result2 = computeBaikovMatrix(G);
    std::cout << "\nBaikov Matrix Result from LabeledGraph:" << std::endl;
    printLabeledGraph(result2);
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    std::cout << "[DEBUG] Baikov ring: " << rString(result2.baikovover) << std::endl;

/*
    // Print the Baikov matrix
    if (result2.baikovmatrix != NULL) {
        std::cout << "Baikov matrix:\n";
        for (int i = 0; i < MATROWS(result2.baikovmatrix); i++) {
            for (int j = 0; j < MATCOLS(result2.baikovmatrix); j++) {
                poly p = MATELEM(result2.baikovmatrix, i + 1, j + 1);
                if (p != NULL) {
                    std::cout << "[" << i << "," << j << "] = " << p_String(p, result2.baikovover) << "\n";
                }
            }
        }
        std::cout << std::endl;
    } */
    
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
    rChangeCurrRing(result2.baikovover);
    ideal M1_direct = computeM1(result2);
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
    rChangeCurrRing(result2.baikovover);
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
    rChangeCurrRing(result2.baikovover);
    ideal M2_direct = computeM2(result2, Nu);
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
    ideal M1 = computeM1(result2);
    if (M1 != NULL) {
        std::cout << "M1 computed successfully\n";
        std::cout << "M1 size: " << IDELEMS(M1) << ", rank: " << M1->rank << std::endl;
    }
    
    // Compute M2
    std::cout << "Computing M2...\n";
    ideal M2 = computeM2(result2, Nu);
    if (M2 != NULL) {
        std::cout << "M2 computed successfully\n";
        std::cout << "M2 size: " << IDELEMS(M2) << ", rank: " << M2->rank << std::endl;
    }
    
    // Compute many IBP relations
    std::cout << "Computing many IBP relations...\n";
    SetIBP ibp = computeManyIBP(result2, setNu);
    if (ibp.IBP != NULL) {
        std::cout << "IBP relations computed successfully\n";
        std::cout << "Number of IBP relations: " << ibp.IBP->nr + 1 << std::endl;
    }
    // Cleanup in reverse order of creation
    std::cout << "[DEBUG] Starting cleanup\n";
    
    // First cleanup IBP-related objects
    if (M1_direct != NULL) {
        std::cout << "[DEBUG] Cleaning up M1_direct module\n";
        id_Delete((ideal*)&M1_direct, result2.baikovover);
    }
    
    if (M2_direct != NULL) {
        std::cout << "[DEBUG] Cleaning up M2_direct module\n";
        id_Delete((ideal*)&M2_direct, result2.baikovover);
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
    
    if (result2.baikovmatrix != NULL) {
        std::cout << "[DEBUG] Cleaning up Baikov matrix\n";
        if (result2.baikovover != NULL) {
            rChangeCurrRing(result2.baikovover);
            id_Delete((ideal*)&result2.baikovmatrix, result2.baikovover);
            rChangeCurrRing(cleanupRing);
        }
    }
    
    if (result2.baikovover != NULL) {
        std::cout << "[DEBUG] Cleaning up Baikov ring\n";
        if (result2.baikovover != cleanupRing) {
            rKill(result2.baikovover);
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