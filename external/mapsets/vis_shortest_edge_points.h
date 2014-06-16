#pragma once

#include "test_visibility.h"
#include "graph_algorithms.h"
#include "visibility_utils.h"
#include "visibility.h"

#include "time_utils.h"
#include "debug_utils.h"

class VisShortestEdgePoints: public TreePointsAlgorithm
{
public:
	map<string, SegmentTree*> BuildTrees(map<string, vector<Point> >& pointsets)
	{
		map<string, SegmentTree*> trees;
		map<Point, int> colors;
		int index = 0;
		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			string cluster = (*iter).first;
			trees[cluster] = new SegmentTree();

			vector<Point>& points = (*iter).second;
			for (int i = 0; i < (int)points.size(); i++)
				colors[points[i]] = index++;
		}

		int t = 0;
		while (true)
		{
			bool progress = chooseBestEdge(pointsets, trees, colors, t);
			t++;
			if (!progress) break;
		}

		//OutputDebugTrees(pointsets, trees, "trees-edge.svg");
		return trees;
	}

	bool chooseBestEdge(map<string, vector<Point> >& pointsets, map<string, SegmentTree*>& trees, map<Point, int>& colors, int t)
	{
		double minLen = INF;
		Point bestI, bestJ;
		string bestCluster;
		vector<Segment> bestSegs;

		//OutputDebugTrees(pointsets, trees, "tree" + int2String(t) + ".svg");
		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			string cluster = (*iter).first;
			vector<Point>& points = (*iter).second;

			vector<Segment> obstacles = GetTreeObstacles(trees, cluster);

			//vis graph computation
			VisibilityGraph visGraph(points, obstacles);

			//distances between pairs
			set<int> checkedColors;
			for (int i = 0; i < (int)points.size(); i++)
			{
				int colorI = colors[points[i]];
				if (checkedColors.find(colorI) != checkedColors.end()) continue;

				pair<VD, VI> dres = RunDijkstra(visGraph, i, trees[cluster], minLen);
				checkedColors.insert(colorI);

				for (int j = i+1; j < (int)points.size(); j++)
				{
					if (colorI == colors[points[j]]) continue;

					if (dres.first[j] < minLen)
					{
						minLen = dres.first[j];
						bestSegs = ExtractVisibilityPath(visGraph, dres.second, i, j);
						bestCluster = cluster;
						bestI = points[i];
						bestJ = points[j];
					}
				}
			}
		}

		if (minLen >= INF) return false;

		SegmentTree* bestTree = trees[bestCluster];
		for (int i = 0; i < (int)bestSegs.size(); i++)
			if (!bestTree->contains(bestSegs[i])) 
				bestTree->append(bestSegs[i]);

		//merging colors
		int oldColor = colors[bestJ];
		int newColor = colors[bestI];
		vector<Point>& bestPoints = pointsets[bestCluster];
		for (int i = 0; i < (int)bestPoints.size(); i++)
			if (colors[bestPoints[i]] == oldColor) colors[bestPoints[i]] = newColor;

		//OutputTimeInfo("best pair: (%.2lf, %.2lf)  (%.2lf, %.2lf)", bestI.x, bestI.y, bestJ.x, bestJ.y);
		return true;
	}

	pair<VD, VI> RunDijkstra(const VisibilityGraph& visGraph, int source, SegmentTree* tree, double upperBound)
	{
		VI dNodes;
		VVI dEdges;
		VVD dDistances;
		for (int i = 0; i < (int)visGraph.nodes.size(); i++)
		{
			dNodes.push_back(i);
			dEdges.push_back(VI());
			dDistances.push_back(VD());

			for (int j = 0; j < (int)visGraph.edges[i].size(); j++)
			{
				int adj = visGraph.edges[i][j];

				Point p1 = visGraph.nodes[i].p;
				Point p2 = visGraph.nodes[adj].p;

				dEdges[i].push_back(adj);
				double dist;
				if (tree->contains(Segment(p1, p2))) dist = EPS;
				else dist = p1.Distance(p2);

				dDistances[i].push_back(dist);
			}
		}

		return Dijkstra2(dNodes, dEdges, dDistances, source, upperBound);
	}

	pair<VD, VI> Dijkstra2(const VI& nodes, const VVI& edges, const VVD& distances, int source, double upperBound)
	{
		VI used = VI(nodes.size(), 0);
		VI parent = VI(nodes.size(), -1);
		VD dist(nodes.size(), INF);
		dist[source] = 0;

		typedef pair<double, int> QE;
		set<QE> q;
		q.insert(make_pair(0.0, source));
 
		while (!q.empty())
		{
			//extract min
			int now = (*q.begin()).second;
			q.erase(q.begin());
			used[now] = 1;
			if (dist[now] > upperBound) continue;

			//update neighbors
			for (int i = 0; i < (int)edges[now].size(); i++)
			{
				int next = edges[now][i];
				if (used[next]) continue;

				double newDist = dist[now] + distances[now][i];

				if (dist[next] > newDist)
				{
					if (dist[next] != INF)
						q.erase(q.find(make_pair(dist[next], next)));

					dist[next] = newDist;
					parent[next] = now;
					q.insert(make_pair(dist[next], next));
				}
			}
		}

		return make_pair(dist, parent);
	}

	vector<Segment> ExtractVisibilityPath(const VisibilityGraph& visGraph, const VI& parent, int s, int t)
	{
		//extract optimal path
		assert(parent[t] != -1);
		vector<Segment> res;
		while (t != s)
		{
			res.push_back(Segment(visGraph.nodes[t].p, visGraph.nodes[parent[t]].p));
			t = parent[t];
		}

		return res;
	}


};
