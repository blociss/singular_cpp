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
LabeledGraph labelGraph(const Graph& G);
ideal balancingIdeal(const LabeledGraph& G);
LabeledGraph eliminateVariables(LabeledGraph G);
ideal propagators(const LabeledGraph& G);
lists ISP(const LabeledGraph& G);
void removeElimVars(LabeledGraph& G);
// Compute Baikov matrix for a labeled graph
/**
 * @brief Computes the Baikov matrix for a labeled graph
 * @param G A labeled graph where redundant variables have been eliminated
 * @return A labeled graph with the Baikov matrix computed
 */
LabeledGraph computeBaikovMatrix(const LabeledGraph& G);
// Module computation functions - using ideal for now as placeholder
ideal computeM1(const LabeledGraph& G);
ideal computeM2(const LabeledGraph& G, lists L);
SetIBP computeIBP(const LabeledGraph& G, lists L);

// Helper functions
Net netList(lists L1);
void printNet(const Net& N);
Net catNet(const Net& N, const Net& M);
void printMat(matrix M);
// Function declarations
lists emptyString(int width);
Graph makeGraph(lists vertices, lists edges);
LabeledGraph makeLabeledGraph(lists vertices, lists edges, ring R, lists labels);
void printGraph(Graph G);
lists createIntList(int* values, int size);
lists createEdgeList(int edges[][2], int size, int singleEdges[], int singleSize);
void printLabeledGraph(LabeledGraph G);
ideal balancingIdeal(const LabeledGraph& G);
LabeledGraph substituteGraph(LabeledGraph G, poly lhs, poly rhs);
LabeledGraph eliminateVariables(LabeledGraph G);
ring removeVariable(ring R, int j);

#endif // FEYNMAN_H
