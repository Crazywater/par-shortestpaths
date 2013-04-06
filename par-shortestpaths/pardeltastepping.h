#ifndef PARDELTASTEPPING_H
#define PARDELTASTEPPING_H

#include "shortestpath.h"
#include "reqentry.h"
#include "dsthread.h"
#include <vector>

class ParDeltaStepping: public ShortestPath
{
public:
    ParDeltaStepping();
    ~ParDeltaStepping();

	virtual void setNumThreads(int numThreads);
    virtual void initialize(Graph &) override {};
    virtual weight_t process(Graph &graph, vertex_idx_t from, vertex_idx_t to);

private:
    vector<Vertex>* vertices;
    // tentative distances
    vector<weight_t> tent;
    // worker threads
    vector<DSThread*> workers;

    // partitions the data into heavy and light edges
    void partition(delta_t delta);
    // estimates the delta value just like the parallel BGL
    delta_t estimateDelta();
    // heavy and light edges according to the paper (> or <= delta)
    vector<vector<Edge> > heavy;
    vector<vector<Edge> > light;
};

#endif // PARDELTASTEPPING_H
