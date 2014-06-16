#pragma once

#include "common.h"
#include "svg_utils.h"
#include "segment.h"
#include "segment_tree.h"
#include "graph.h"

void TestClosestPoints();
void TestSegmentIntersections();

class DebugCurve
{
public:
	Segment segment;
	Color color;
	double width;

	DebugCurve(const Segment& segment): segment(segment), color(Color::BLACK), width(1.0) {}
	DebugCurve(const Segment& segment, const Color& color): segment(segment), color(color), width(1.0) {}
	DebugCurve(const Segment& segment, const Color& color, double width): segment(segment), color(color), width(width) {}
};

void OutputDebugCurves(const string& filename, const vector<DebugCurve>& curves);

Color ChooseClusterColor(const string& clusterId);

void OutputDebugTrees(const Graph& g, const map<string, SegmentTree*>& trees, const string& filename);
void OutputDebugTrees(map<string, vector<Point> >& pointsets, const map<string, SegmentTree*>& trees, const string& filename);
