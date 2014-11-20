#pragma once

#include "common/graph/dot_graph.h"

#include "segment_tree.h"

vector<Segment> GetBoundaryObstacles(const DotGraph& g, const string& avoidClusterId, double marginCoef);

vector<Segment> GetTreeObstacles(const map<string, vector<Segment> >& trees, const string& avoidClusterId);

vector<Segment> GetTreeObstacles(const map<string, SegmentTree*>& trees, const string& avoidClusterId);
