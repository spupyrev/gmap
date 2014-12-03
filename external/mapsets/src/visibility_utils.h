#pragma once

#include "common/graph/dot_graph.h"

#include "segment_set.h"

vector<Segment> GetBoundaryObstacles(const DotGraph& g, const string& avoidClusterId, double marginCoef);

vector<Segment> GetTreeObstacles(const map<string, SegmentSet*>& trees, const string& avoidClusterId);

Rectangle SegmentBoundingBox(const Segment& seg);
