#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>
#include "feynman.h"
#include "polys/ext_fields/transext.h"
int main() {
    siInit((char*)"/home/atraore/Singular4/lib/libSingular.so");
//  graph G = makeGraph(list(1,2,3,4),list(list(1,3),list(1,2),list(2,4),list(3,4),list(1),list(2),list(3),list(4)));

/*  int verticesData[] = {1, 2, 3, 4};
    lists vertices = createIntList(verticesData, 4);

    int boundedEdges[][2] = {{1,3},{1,2},{2,4},{3,4}};
    int unboundedEdges[] = {1, 2, 3, 4};
    lists edges = createEdgeList(boundedEdges, 4, unboundedEdges, 4);*/

    int verticesData[] = {1, 2, 3, 4, 5, 6};
    lists vertices = createIntList(verticesData, 6);

    int boundedEdges[][2] = {{1,2},{3,6},{4,5},{1,6},{2,3},{5,6},{3,4}};
    int unboundedEdges[] = {1, 2, 5, 4};
    lists edges = createEdgeList(boundedEdges, 7, unboundedEdges, 4); 
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    LabeledGraph lg = makeLabeledGraph(vertices, edges, currRing, NULL, currRing);
    std::cout << "[DEBUG] printing labelgraph\n";
    printLabeledGraph(lg);
    Graph G = makeGraph(vertices, edges);
    LabeledGraph lG = labelGraph(G, 0);
     std::cout<<"******computing eliminations******\n";
    LabeledGraph lG1 = eliminateVariables(lG);
    printLabeledGraph(lG1);
    ideal J = feynmanDenominators(lG1);
    printIdeal(J);
    std::cout<<"******computing propagators******\n";
    ideal I = propagators(lG1);
    printIdeal(I);
    std::cout << "*****[DEBUG] Computing ISP...\n";
    ideal ispIdeal = ISP(lG1);
    printIdeal(ispIdeal);
    std::cout<<"******computing Baikov matrix******\n";
    LabeledGraph B = computeBaikovMatrix(lG1);
    std::cout<<"[DEBUG] Printing Baikov matrix\n";
    printLabeledGraph(B);
    // Cleanup (remember to clean up rings, ideals, and lists properly)
    rKill(lG.over);
    return 0;
}
