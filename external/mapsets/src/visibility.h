#pragma once

#include "common/common.h"
#include "common/geometry/point.h"
#include "common/geometry/segment.h"
#include "common/graph/dot_graph.h"

#include "segment_set.h"

struct VisibilityVertex
{
	int index;
	Point p;
	Point leftP, rightP;
	//real visibility vertices correspond to the vertices of the original graph
	bool real;

	VisibilityVertex() {}
	VisibilityVertex(Point p, bool real, int index): index(index), p(p), real(real) {}
};

class VisibilityGraph
{
private:
	VisibilityGraph(const VisibilityGraph&);
	VisibilityGraph& operator = (const VisibilityGraph&);

	void Initialze(const vector<Point>& points, const vector<Segment>& obstacles, bool fullVisibility);
	vector<VisibilityVertex> CreateVisibilityVertices(const vector<Point>& p, const vector<Segment>& obstacles);
	vector<vector<int> > CreateVisibilityEdges(const vector<VisibilityVertex>& vis, const vector<Segment>& obstacles, bool fullVisibility);

public:
	vector<VisibilityVertex> nodes;
	VVI edges;

	VisibilityGraph(vector<VisibilityVertex>& nodes, VVI edges): nodes(nodes), edges(edges) {}
	VisibilityGraph(const vector<Point>& points, const vector<Segment>& obstacles) 
	{
		Initialze(points, obstacles, false);
	}

	VisibilityGraph(const vector<Point>& points, const vector<Segment>& obstacles, bool fullVisibility) 
	{
		Initialze(points, obstacles, fullVisibility);
	}

};


class CESTAlgorithm
{
public:
	map<string, SegmentSet*> BuildTrees(DotGraph& g);

	double TreeLength(map<string, SegmentSet*>& trees) const
	{
		double res = 0;
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
			res += (*iter).second->Length();;
		return res;
	}

	void CheckTreeConnected(SegmentSet* tree, vector<Point>& points, vector<Segment>& obstacles) const
	{
		VisibilityGraph visGraph(points, obstacles, false);

		VVI edges;
		for (int i = 0; i < (int)visGraph.nodes.size(); i++)
		{
			VI curEdges;
			for (int j = 0; j < (int)visGraph.edges[i].size(); j++)
			{
				int adj = visGraph.edges[i][j];
				Point p1 = visGraph.nodes[i].p;
				Point p2 = visGraph.nodes[adj].p;

				Segment seg(p1, p2);
				if (tree->contains(seg)) curEdges.push_back(adj);
			}

			edges.push_back(curEdges);
		}

		VI used = VI(edges.size(), 0);
		int cc = 0;
		for (int i = 0; i < (int)points.size(); i++)
			if (!used[i]) {bfs(i, used, edges); cc++;}

		assert(cc == 1);
	}

	void bfs(int now, VI& used, VVI& edges) const
	{
		used[now] = 1;
		for (int i = 0; i < (int)edges[now].size(); i++)
			if (!used[edges[now][i]]) bfs(edges[now][i], used, edges);
	}

};
