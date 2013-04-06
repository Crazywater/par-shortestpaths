#include "dsthread.h"
#include "pardeltastepping.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#ifndef _MSC_VER
#include <sched.h>
#endif

DSThread::DSThread(delta_t delta_,
                   vector<Vertex>* vertices_,
                   vector<weight_t>* tent_,
                   vector<DSThread*>* workers_,
                   size_t me_,
                   Barrier* barr_,
                   vector<vector<Edge> > *light_,
                   vector<vector<Edge> > *heavy_):
    delta(delta_),
    vertices(vertices_),
    tent(tent_),
    workers(workers_),
    me(me_),
    threadObj(nullptr),
    barr(barr_),
    light(light_),
    heavy(heavy_)
{
    workLists.resize(workers->size());
}

DSThread::~DSThread()
{
    if (threadObj != nullptr && threadObj->joinable())
    {
        delete threadObj;
    }
}

void DSThread::enqueue(const ReqEntry& reqEntry, size_t who)
{
    workLists[who].push_back(reqEntry);
}

void DSThread::start()
{
    if (threadObj != nullptr)
    {
        delete threadObj;
    }
    threadObj = new thread([this]{this->run();});
}

void DSThread::join()
{
    assert(threadObj != nullptr);
    assert(threadObj->joinable());
    threadObj->join();
}

void DSThread::run()
{
#ifdef _MSC_VER
	::SetThreadPriority(nullptr, THREAD_PRIORITY_TIME_CRITICAL);
#else
    // pin to CPU "me"
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(me, &mask);
    int pinned = sched_setaffinity(0, sizeof(mask), &mask);
    if (pinned < 0)
    {
        cout << "could not set affinity for process " << me << endl;
    }
#endif

    // do actual work
    currentStep = 0;
    chunksize = ceil((double)vertices->size()/workers->size());
    do
    {
        doStep();
        computeMinBucket();
        barrier();
        currentStep = getTotalMinBucket();
    } while (currentStep >= 0);
}

void DSThread::doStep() {
    unordered_set<vertex_idx_t> S;
    do
    {
        // light edges
        finishedThreads = 0;
        if (B.size() > (unsigned long) currentStep)
        {
            unordered_set<vertex_idx_t>* Bi = &B[currentStep];
            // work distribution phase
            S.insert(Bi->begin(), Bi->end());
            distribute(*Bi, false);
            Bi->clear();
        }
        barrier();
        // relaxation phase
        processWorkList();
        // check if everything is done
        if (B.size() <= (unsigned long) currentStep || B[currentStep].empty())
        {
            finishedThreads++;
        }
        barrier();
    } while (finishedThreads < workers->size());
    // heavy edges: work distribution and relaxation
    distribute(S, true);
    barrier();
    processWorkList();
}

void DSThread::processWorkList()
{
    for (vector<ReqEntry>& workList : workLists)
    {
        for (const ReqEntry& req : workList)
        {
            relax(req.prev, req.v, req.x);
        }
        workList.clear();
    }
}

template <class BaseSet>
void DSThread::distribute(BaseSet& set, bool isHeavy)
{
    for(vertex_idx_t v : set)
    {
        auto& vSet = (*vertices)[v].out_edges;

        for (const Edge& e : vSet)
        {
            if ((e.weight > delta) ^ isHeavy)
            {
                continue;
            }
            weight_t newDistance = e.weight + (*tent)[v];
            // only create a new relaxation request if the new distance would be smaller
            if (newDistance < (*tent)[e.target])
            {
                size_t who = whose(e.target);
                // create a relaxation request
                ReqEntry entry = {v, e.target, newDistance};
                // forward it to whoever is responsible (might be me)
                (*workers)[who]->enqueue(entry, me);
            }
        }
    }
}

void DSThread::relax(vertex_idx_t prev, vertex_idx_t v, weight_t newDistance)
{
    weight_t oldDistance = (*tent)[v];
    if (newDistance < oldDistance)
    {
        if (oldDistance != INFINITE_WEIGHT)
        {
            // remove from bucket
            B[oldDistance/delta].erase(v);
        }
        // insert into new bucket
        size_t newBucket = newDistance/delta;
        if (B.size() <= newBucket) {
            B.resize(newBucket + 1);
        }
        B[newBucket].insert(v);
        // set new result
        (*tent)[v] = newDistance;
        (*vertices)[v].prev = prev;
    }
}

size_t DSThread::whose(vertex_idx_t v)
{
    // pseudo-random assignment
    return (v * 2654435761) % workers->size();
}

void DSThread::barrier()
{
    barr->wait();
}

void DSThread::computeMinBucket()
{
    minBucket = -1;
    for (size_t i = currentStep; i < B.size(); i++)
    {
        if (!B[i].empty())
        {
            minBucket = i;
            return;
        }
	}
}

int DSThread::getTotalMinBucket()
{
    int totalMinBucket = -1;
    for (size_t i = 0; i < workers->size(); i++)
    {
        int minBucket = (*workers)[i]->minBucket;
        if (minBucket >= 0 && (totalMinBucket == -1 || minBucket < totalMinBucket))
        {
            totalMinBucket = minBucket;
        }
    }
    return totalMinBucket;
}

volatile atomic<size_t> DSThread::finishedThreads;
