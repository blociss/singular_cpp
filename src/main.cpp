#include <singular/Singular/libsingular.h>

// Define the Graph structure
struct Graph {
    lists vertices; // List of vertices
    lists edges;    // List of edges
};

// Function to create a Graph from lists of vertices and edges
Graph makeGraph(lists v, lists e) {
    Graph G;
    G.vertices = v;
    G.edges = e;
    return G;
}

// Helper function to create a list of integers
lists makeIntList(const int* values, int size) {
    lists L = (lists)omAlloc(sizeof(sleftv));
    L->Init();
    for (int i = 0; i < size; i++) {
        L->m[i].rtyp = INT_CMD;           // Type: integer
        L->m[i].data = (void*)(long)values[i]; // Data: integer value
    }
    L->nr = size - 1; // Number of elements (0-based index of last element)
    return L;
}

// Helper function to create a list of edges (each edge is a list of 2 integers)
lists makeEdgeList(const int edges[][2], int size) {
    lists L = (lists)omAlloc(sizeof(sleftv));
    L->Init();
    for (int i = 0; i < size; i++) {
        lists edge = (lists)omAlloc(sizeof(sleftv));
        edge->Init();
        edge->m[0].rtyp = INT_CMD;
        edge->m[0].data = (void*)(long)edges[i][0]; // First vertex of edge
        edge->m[1].rtyp = INT_CMD;
        edge->m[1].data = (void*)(long)edges[i][1]; // Second vertex of edge
        edge->nr = 1; // Edge list has 2 elements (index 0 and 1)
        L->m[i].rtyp = LIST_CMD; // Type: list
        L->m[i].data = (void*)edge; // Data: nested list for the edge
    }
    L->nr = size - 1; // Number of edges
    return L;
}

// Example usage
int main() {
    // Initialize Singular (replace with actual path to libSingular.so)
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Create vertices list: [1, 2, 3, 4]
    int verticesData[] = {1, 2, 3, 4};
    lists vertices = makeIntList(verticesData, 4);

    // Create edges list: [[1,3], [1,2], [1,2], [2,4], [3,4], [3,4]]
    int edgesData[][2] = {{1, 3}, {1, 2}, {1, 2}, {2, 4}, {3, 4}, {3, 4}};
    lists edges = makeEdgeList(edgesData, 6);

    // Create the graph
    Graph G = makeGraph(vertices, edges);

    // Cleanup
  

    return 0;
}