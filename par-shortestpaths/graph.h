#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <climits>

using namespace std;

typedef long weight_t;
typedef unsigned long vertex_idx_t;

#define INFINITE_WEIGHT INT_MAX

struct Edge {
    vertex_idx_t target;
    weight_t weight;

    bool operator==(const Edge& other) const {
        return (this == &other);
    }
};

struct Vertex {
    vector<Edge> out_edges;
    vertex_idx_t prev;
};

class Graph {
public:
    Graph();
    vector<Vertex> vertices;
    void resize(size_t numVertices);
    void add_edge(vertex_idx_t from, vertex_idx_t to, weight_t weight);
};

#endif // GRAPH_H
