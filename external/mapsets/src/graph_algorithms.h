#pragma once 

#include "common/common.h"
#include "common/geometry/point.h"

pair<VD, VI> GenericDijkstraMSTAlgorithm(const VI& nodes, const VVI& edges, const VVD& distances, int source, bool isDijkstra);

//returns pair<distances_to_source, parents>
pair<VD, VI> Dijkstra(const VI& nodes, const VVI& edges, const VVD& distances, int source);

//returns pair<distances_to_source, parents>
pair<VD, VI> MinimumSpanningTree(const VI& nodes, const VVI& edges, const VVD& distances, int source);

//returns length of the tree
double LengthMinimumSpanningTree(const VI& nodes, const VVI& edges, const VVD& distances, int source);

//length of minimum spanning tree on a pointset
double LengthMinimumSpanningTree(const vector<Point>& p);
