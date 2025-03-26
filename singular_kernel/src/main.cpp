#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "feynman.h"
#include "polys/ext_fields/transext.h"
int main() {
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
//  graph G = makeGraph(list(1,2,3,4),list(list(1,3),list(1,2),list(2,4),list(3,4),list(1),list(2),list(3),list(4)));

 int verticesData[] = {1, 2, 3, 4};
    lists vertices = createIntList(verticesData, 4);

    int boundedEdges[][2] = {{1,3},{1,2},{2,4},{3,4}};
    int unboundedEdges[] = {1, 2, 3, 4};
    lists edges = createEdgeList(boundedEdges, 4, unboundedEdges, 4);

    /* int verticesData[] = {1, 2, 3, 4, 5, 6};
    lists vertices = createIntList(verticesData, 6);

    int boundedEdges[][2] = {{1,2},{3,6},{4,5},{1,6},{2,3},{5,6},{3,4}};
    int unboundedEdges[] = {1, 2, 3, 4};
    lists edges = createEdgeList(boundedEdges, 7, unboundedEdges, 4); */
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    LabeledGraph lg = makeLabeledGraph(vertices, edges, currRing, NULL, currRing);
    std::cout << "[DEBUG] printing labelgraph\n";
    printLabeledGraph(lg);
    std::cout << "[DEBUG] making graph\n";
    Graph G = makeGraph(vertices, edges);
    LabeledGraph lG = labelGraph(G, 0);
    std::cout << "[DEBUG] Eliminating variables...\n";
    LabeledGraph lG1 = eliminateVariables(lG);
    std::cout << "[DEBUG] Eliminated variables...\n";
    std::cout << "[DEBUG] Computing Feynman denominators...\n";
    ideal J = feynmanDenominators(lG1);
    std::cout << "[DEBUG] Computed Feynman denominators...\n";
    std::cout << "[DEBUG] Printing Feynman denominators...\n";
    printIdeal(J);
    std::cout << "[DEBUG] Computing propagators...\n";
    ideal I = propagators(lG1);
    std::cout << "[DEBUG] Computed propagators...\n";
    std::cout << "[DEBUG] Printing propagators...\n";
    printIdeal(I);
    
    // Cleanup (remember to clean up rings, ideals, and lists properly)
    rKill(lG.over);
    return 0;
}
