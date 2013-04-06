#include "shortestpath.h"
#include "pardeltastepping.h"
#include <iostream>

using namespace std;

int main()
{
    // create a graph
    Graph graph;
    graph.resize(11);
    graph.add_edge(0, 3, 20); // 0->3 has weight 20
    graph.add_edge(3, 10, 5);
    vertex_idx_t from = 0;
    vertex_idx_t to = 10;

    // initialize the algorithm
    ShortestPath* algo = new ParDeltaStepping();
    algo->setNumThreads(32);
    algo->initialize(graph);

    // run it
    weight_t total_weight = algo->process(graph, from, to);
    cout << "Total weight: " << total_weight << endl;

    // extract the path
    if (total_weight == INFINITE_WEIGHT)
    {
        cout << "No path found." << endl;
    } else {
        cout << "Path: ";
        // trace backwards from "to" to "from"
        vertex_idx_t cur = to;
        do
        {
            cout << cur << "<-";
            cur = graph.vertices[cur].prev;
        } while (cur != from);
        cout << from << endl;
    }
}
