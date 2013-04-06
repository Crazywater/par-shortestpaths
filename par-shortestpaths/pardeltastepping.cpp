#include "pardeltastepping.h"
#include <unordered_set>
#include <iostream>
#include <omp.h>

ParDeltaStepping::ParDeltaStepping()
{
}

ParDeltaStepping::~ParDeltaStepping()
{
}

weight_t ParDeltaStepping::process(Graph &graph, vertex_idx_t from, vertex_idx_t to)
{
    vector<Vertex>& v = graph.vertices;
    vertices = &(graph.vertices);
    // initialize distances
    tent.clear();
    tent.resize(v.size(), INFINITE_WEIGHT);
    // estimate the delta value
    delta_t delta = estimateDelta();
    size_t numWorkers = workers.size();
    Barrier barr(numWorkers);
    for (size_t i = 0; i < numWorkers; i++)
    {
        workers[i] = new DSThread(delta, vertices, &tent, &workers, i, &barr, &light, &heavy);
    }
    // relax the first node to 0
    ReqEntry req = {from, from, 0};
    workers[0]->enqueue(req, 0);
    // actual work here
    for (size_t i = 0; i < workers.size(); i++)
    {
        workers[i]->start();
    }
    // wait for finish
    for (size_t i = 0; i < workers.size(); i++)
    {
        workers[i]->join();
    }
    // delete worker threads
    for (size_t i = 0; i < numWorkers; i++)
    {
        delete(workers[i]);
        workers[i] = nullptr;
    }
    return tent[to];
}

delta_t ParDeltaStepping::estimateDelta()
{
    size_t nVertices = (*vertices).size();
    size_t maxdegree = 0;
    weight_t maxweight = 0;
    size_t degree;
    weight_t weight;
    #pragma omp parallel private(degree, weight)
    {
        degree = 0;
        weight = 0;
        // find the maximum degree and weight for this thread
        #pragma omp for
        for (vertex_idx_t i = 0; i < nVertices; i++)
        {
            vector<Edge>& edges = (*vertices)[i].out_edges;
            size_t deg = edges.size();
            if (deg > degree)
            {
                degree = deg;
            }
            for (size_t j = 0; j < deg; j++)
            {
                weight_t w = edges[j].weight;
                if (w > weight)
                {
                    weight = w;
                }
            }
        }
        // find the maximum degree and weight across all threads
        #pragma omp critical
        {
            if (degree > maxdegree)
            {
                maxdegree = degree;
            }
            if (weight > maxweight)
            {
                maxweight = weight;
            }
        }
    }
    delta_t delta = (delta_t)((double)maxweight/maxdegree);
    if (delta <= 0)
    {
        delta = 1;
    }
    return delta;
}

void ParDeltaStepping::setNumThreads(int numThreads)
{
    for (size_t i = 0; i < workers.size(); i++)
    {
        DSThread* t = workers[i];
        if (t != nullptr)
        {
            delete t;
        }
    }
    workers.resize(numThreads, nullptr);
    omp_set_num_threads(numThreads);
}
