#include "debug_utils.h"

void OutputDebugCurves(const string& filename, const vector<DebugCurve>& curves)
{
	FILE* f = fopen(filename.c_str(), "w");
	writeSVGHead(f);

	for (int i = 0; i < (int)curves.size(); i++)
	{
		drawSVGLine(f, curves[i].segment.first, curves[i].segment.second, curves[i].color, curves[i].width);
		drawSVGCircle(f, curves[i].segment.first, curves[i].width * 1.5, curves[i].color);
		drawSVGCircle(f, curves[i].segment.second, curves[i].width * 1.5, curves[i].color);
	}

	writeSVGTail(f);
	fclose(f);
}

Color ChooseClusterColor(const string& clusterId)
{
	if (clusterId == "1") return Color::BLUE;
	else if (clusterId == "2") return Color::RED;
	else if (clusterId == "3") return Color::GREEN;
	else if (clusterId == "4") return Color::ORANGE;
	else if (clusterId == "5") return Color::CYAN;
	else if (clusterId == "6") return Color::PURPLE;
	else if (clusterId == "red") return Color::RED;
	else if (clusterId == "blue") return Color::BLUE;

	return Color::BLACK;
}

void OutputDebugTrees(const Graph& g, const map<string, SegmentTree*>& trees, const string& filename)
{
	vector<DebugCurve> dc;
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentTree* segs = (*iter).second;

		Color color = ChooseClusterColor(clusterId);
		for (int i = 0; i < segs->count(); i++)
			dc.push_back(DebugCurve(segs->get(i), color, 0.3));
	}

	vector<Segment> boundaryObstacles;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		vector<Segment> boundary = g.nodes[i]->getBoundary(1.0);
		boundaryObstacles.insert(boundaryObstacles.end(), boundary.begin(), boundary.end());
	}
	
	for (int i = 0; i < (int)boundaryObstacles.size(); i++)
		dc.push_back(DebugCurve(boundaryObstacles[i], Color::BLACK));

	OutputDebugCurves(filename, dc);
}

void OutputDebugTrees(map<string, vector<Point> >& pointsets, const map<string, SegmentTree*>& trees, const string& filename)
{
	vector<DebugCurve> dc;
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentTree* tree = (*iter).second;

		Color color = ChooseClusterColor(clusterId);
		for (int i = 0; i < tree->count(); i++)
			dc.push_back(DebugCurve(tree->get(i), color, 0.3));
	}

	for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
	{
		string clusterId = (*iter).first;
		vector<Point> points = (*iter).second;
		Color color = ChooseClusterColor(clusterId);

		for (int i = 0; i < (int)points.size(); i++)
			dc.push_back(DebugCurve(Segment(points[i], points[i]), color));
	}

	OutputDebugCurves(filename, dc);
}
