#include <graph.h>

Graph::Graph()
{
}

void Graph::add_edge(vertex_idx_t from, vertex_idx_t to, weight_t weight)
{
    if (vertices.size() <= from)
    {
        vertices.resize(from + 1);
    }
    if (vertices.size() <= to)
    {
        vertices.resize(to + 1);
    }
    Vertex& f = vertices[from];
    Edge e = {to, weight};
    f.out_edges.push_back(e);
}

void Graph::resize(size_t numVertices)
{
    vertices.resize(numVertices);
}
