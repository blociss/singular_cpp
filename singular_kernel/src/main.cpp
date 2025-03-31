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

   // graph G = makeGraph(list(1,2,3,4,5,6),list(list(6,1),list(4,6),list(1,2),list(3,5),list(4,3),list(2,5),list(5,6),list(1),list(2),list(3),list(4)));

    int verticesData[] = {1, 2, 3, 4, 5, 6};
    lists vertices = createIntList(verticesData, 6);

    int boundedEdges[][2] = {{6,1},{4,6},{1,2},{3,5},{4,3},{2,5},{5,6},{1},{2},{3},{4}};
    int unboundedEdges[] = {1, 2, 3, 4};
    lists edges = createEdgeList(boundedEdges, 7, unboundedEdges, 4); 
    std::cout << "[DEBUG] Current ring: " << rString(currRing) << std::endl;
    LabeledGraph lg = makeLabeledGraph(vertices, edges, currRing, NULL, currRing);
    std::cout << "[DEBUG] printing labelgraph\n";
    printLabeledGraph(lg);
    Graph G = makeGraph(vertices, edges);
     LabeledGraph lG = labelGraph(G, 0);
     /*
    ring RR = lG.over;
    ring RRp = lG.overpoly;
    std::cout<<"[DEBUG] printing ring RR=lG.over :"<<rString(RR)<<std::endl;
    std::cout<<"[DEBUG] printing ring RR=lG.overpoly :"<<rString(RRp)<<std::endl;
    std::cout<<"******computing removeVariable(RR, 1)******\n"; */
    //ring RR1    = removeVariable(RR, 1);
    //ring RR2 = removeParameter(RR, 4);
   // ring RRp1 = removeVariable(RRp, 1);
    //std::cout<<"[DEBUG] printing ring from removeVariable\n";
    //std::cout << "[DEBUG] RR1.over and RR1.overpoly ring: " << rString(RR1)  << std::endl;


      LabeledGraph lG1 = eliminateVariables(lG);
 std::cout<<"printing ring from eliminateVariables\n";
 printLabeledGraph(lG1);

    std::cout << "[DEBUG] lG1.over and lG1.overpoly ring: " << rString(lG1.over) << " \n" << rString(lG1.overpoly) << std::endl;
    std::cout<<"******removing elimination variables in main.cpp******\n";
    LabeledGraph lG2 = removeElimVars(lG1);
 
    std::cout<<"printing ring from removeElimVars\n";
    std::cout << "[DEBUG] lG2.over and lG2.overpoly ring: " << rString(lG2.over) << " \n" << rString(lG2.overpoly) << std::endl;
   std::cout<<"printing labeled graph from removeElimVars\n";
    printLabeledGraph(lG2);
    /*
    std::cout<<"******computing feynman denominators******\n"; 
   ideal J = feynmanDenominators(lG2);
    printIdeal(J);
    std::cout<<"******computing propagators******\n";
    ideal I = propagators(lG2);
    printIdeal(I);
    std::cout << "*****[DEBUG] Computing ISP...\n";
    ideal ispIdeal = ISP(lG1);
    printIdeal(ispIdeal); 
    std::cout<<"******computing Baikov matrix in main.cpp******\n";
    std::cout<<"[DEBUG] type of G: "<<typeid(G).name()<<std::endl;
    LabeledGraph B = computeBaikovMatrix(G);
    std::cout<<"[DEBUG] Printing Baikov matrix\n";
    printLabeledGraph(B); */
    // Cleanup (remember to clean up rings, ideals, and lists properly)
   // rKill(lG.over);
    return 0;
}
