#pragma once

#include "common.h"
#include "segment.h"
#include "segment_tree.h"
#include "graph.h"

vector<Segment> GetBoundaryObstacles(const Graph& g, const string& avoidClusterId, double marginCoef);

vector<Segment> GetTreeObstacles(const map<string, vector<Segment> >& trees, const string& avoidClusterId);

vector<Segment> GetTreeObstacles(const map<string, SegmentTree*>& trees, const string& avoidClusterId);
