#ifndef SHORTESTPATH_H
#define SHORTESTPATH_H

#include <graph.h>
#include <string>

class ShortestPath
{
public:
    ShortestPath();
	virtual ~ShortestPath() = 0;

	virtual void setNumThreads(int numThreads) = 0;
    virtual void initialize(Graph &graph) = 0;
    virtual weight_t process(Graph& graph, vertex_idx_t from, vertex_idx_t to) = 0;
};

#endif // SHORTESTPATH_H
