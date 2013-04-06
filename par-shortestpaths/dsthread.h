#ifndef DSTHREAD_H
#define DSTHREAD_H

#include <unordered_set>
#include <vector>
#include <atomic>
#include <thread>
#include "graph.h"
#include "reqentry.h"
#include "barrier.h"

typedef weight_t delta_t;

class DSThread
{
public:
    DSThread(delta_t delta_,
             vector<Vertex>* vertices_,
             vector<weight_t>* tent_,
             vector<DSThread*>* workers_,
             size_t me_,
             Barrier* barr_,
             vector<vector<Edge> > *light_,
             vector<vector<Edge> > *heavy_);
    ~DSThread();

    // enqueue a relaxation request (who is the enqueuer)
    void enqueue(const ReqEntry& entry, size_t who);
    void start();
    void join();

private:
    delta_t delta;
    vector<Vertex>* vertices;
    // tentative distances
    vector<weight_t>* tent;
    // bucket data structure
    vector<unordered_set<vertex_idx_t> > B;
    // each thread keeps #threads worklists
    // so all threads can insert relaxation requests in parallel
    vector<vector<ReqEntry> > workLists;
    // all workers, so that we can communicate with them
    vector<DSThread*>* workers;
    // the current step in the algorithm
    int currentStep;
    // how many threads have finished with their light edge relaxations
    static volatile atomic<size_t> finishedThreads;
    // my thread-id
    size_t me;
    // my c++11 thread object
    thread* threadObj;
    // barrier used for barrier() sync among workers
    Barrier* barr;
    // current minimum bucket
    int minBucket;
    // light and heavy edges if we don't use on-the-fly-partitioning
    vector<vector<Edge> > *light;
    vector<vector<Edge> > *heavy;
    // chunksize for partitioning the graph
    size_t chunksize;

    // implementation of start()
    void run();
    // perform a step in the algorithm
    void step(size_t i);
    // compute the minimum occupied bucket in this thread and store it in minBucket
    void computeMinBucket();
    // get the total minimum bucket across all threads
    int getTotalMinBucket();
    // distribute work among worker threads
    template <class BaseSet>
    void distribute(BaseSet& set, bool isHeavy);
    // perform a single relaxation request
    void relax(vertex_idx_t prev, vertex_idx_t v, weight_t newDistance);
    // perform a barrier-sync between all workers
    void barrier();
    // perform a step in the algorithm
    void doStep();
    // process the entire worklist and empty it
    void processWorkList();
    // who is responsible for relaxing a certain vertex
    size_t whose(vertex_idx_t v);
};

#endif // DSTHREAD_H
