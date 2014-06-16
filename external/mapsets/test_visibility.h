#pragma once

#include "common.h"
#include "svg_utils.h"
#include "segment.h"
#include "segment_tree.h"
#include "graph.h"
#include "visibility.h"
#include "visibility_utils.h"

class TreePointsAlgorithm
{
public:
	map<string, SegmentTree*> BuildTrees(map<string, vector<Point> >& pointsets);

	double TreeLength(map<string, SegmentTree*>& trees) const
	{
		double res = 0;
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
			res += (*iter).second->Length();;
		return res;
	}

	void CheckTrees(map<string, SegmentTree*>& trees, map<string, vector<Point> >& pointsets) const
	{
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
		{
			(*iter).second->CheckTree();

			string cluster = (*iter).first;
			vector<Segment> obstacles = GetTreeObstacles(trees, cluster);
			CheckTreeConnected((*iter).second, pointsets[cluster], obstacles);
		}
	}

	void CheckTreeConnected(SegmentTree* tree, vector<Point>& points, vector<Segment>& obstacles) const
	{
		VisibilityGraph visGraph(points, obstacles);

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

void TestVisibilityAlgorithms();
