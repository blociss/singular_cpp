#ifndef FEYNMAN_H
#define FEYNMAN_H

#include <Singular/libsingular.h>
#include <vector>
#include <string>

// Forward declarations
struct Graph;
struct LabeledGraph;
struct Net;
struct OneIBP;
struct SetIBP;
struct OneSector;

// Graph structure
struct Graph {
    lists vertices;
    lists edges;
};

// LabeledGraph structure
struct LabeledGraph {
    lists vertices;
    lists edges;
    ring over;
    lists labels;
    ring overpoly;
    lists elimvars;
    ring baikovover;
    matrix baikovmatrix;
};

// Net structure for pretty printing
struct Net {
    lists rows;
};

// IBP structures
struct OneIBP {
    lists c;
    lists i;
};

struct SetIBP {
    ring over;
    lists seed;
    lists IBP;
};

struct OneSector {
    lists lab;
    lists sectorMap;
    lists targetInts;
};

// IBP computation functions
ideal computeM1(const LabeledGraph& G);
ideal computeM2(const LabeledGraph& G, lists Nu);
SetIBP computeManyIBP(const LabeledGraph& G, lists setNu);
// Struct definitions mimicking Singular's newstruct



// Core functions
Graph makeGraph(lists vertices, lists edges);
LabeledGraph makeLabeledGraph(lists vertices, lists edges);
void printGraph(const Graph& G);
LabeledGraph labelGraph(Graph G, int ch);
ideal balancingIdeal(const LabeledGraph& G);
LabeledGraph eliminateVariables(LabeledGraph G);
ideal propagators(const LabeledGraph& G);
ideal feynmanDenominators(const LabeledGraph& G);
ideal ISP(const LabeledGraph& G);
// Compute Baikov matrix for a labeled graph
/**
 * @brief Computes the Baikov matrix for a labeled graph
 * @param G A labeled graph where redundant variables have been eliminated
 * @return A labeled graph with the Baikov matrix computed
 */
LabeledGraph computeBaikovMatrix(const Graph& G0);
LabeledGraph computeBaikovMatrix(const LabeledGraph& G0);
// Module computation functions - using ideal for now as placeholder
ideal computeM1(const LabeledGraph& G);
ideal computeM2(const LabeledGraph& G, lists L);
SetIBP computeIBP(const LabeledGraph& G, lists L);

// Helper functions
int size(lists L);
Net netList(lists L1);
void printNet(const Net& N);
Net catNet(const Net& N, const Net& M);
void printMat(matrix M);
// Function declarations
lists emptyString(int width);
Graph makeGraph(lists vertices, lists edges);
LabeledGraph makeLabeledGraph(lists vertices, lists edges, ring R, lists labels, ring Rpoly);

lists createIntList(int* values, int size);
lists createEdgeList(int edges[][2], int size, int singleEdges[], int singleSize);
void printLabeledGraph(const LabeledGraph& G);
void printIdeal(const ideal I);
ideal balancingIdeal(const LabeledGraph& G);
LabeledGraph substituteGraph(LabeledGraph G, poly lhs, poly rhs);
LabeledGraph eliminateVariables(LabeledGraph G);
ring removeVariable(ring R, int j);
ring removeParameter(ring R, int j);
LabeledGraph removeElimVars(const LabeledGraph& G);
// Create a ring with specified variables and extension field
ring createRing(char **extNames, int extCount, char **varNames, int varCount, rRingOrder_t varOrdering);

// Helper function to get the first variable index with non-zero exponent
int getVariableIndex(poly p, ring r);

#endif // FEYNMAN_H
