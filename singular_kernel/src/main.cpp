#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "feynman.h"
#include "polys/ext_fields/transext.h"
int main() {
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");

    int verticesData[] = {1, 2, 3, 4, 5, 6};
    lists vertices = createIntList(verticesData, 6);

    int boundedEdges[][2] = {{1,2},{3,6},{4,5},{1,6},{2,3},{5,6},{3,4}};
    int unboundedEdges[] = {1, 2, 5, 4};
    lists edges = createEdgeList(boundedEdges, 7, unboundedEdges, 4);

    Graph G = makeGraph(vertices, edges);
    LabeledGraph lG = labelGraph(G, 0);
    rChangeCurrRing(lG.over);
    std::cout << "[DEBUG] Eliminating variables...\n";
    LabeledGraph lG1 = eliminateVariables(lG);
    std::cout << "[DEBUG] Eliminated variables...\n";
    // Cleanup (remember to clean up rings, ideals, and lists properly)
    rKill(lG.over);
    return 0;
}
