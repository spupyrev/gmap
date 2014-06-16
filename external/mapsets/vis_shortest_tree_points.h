#pragma once

#include "test_visibility.h"
#include "graph_algorithms.h"
#include "visibility_utils.h"
#include "visibility.h"

#include "time_utils.h"
#include "debug_utils.h"

class VisIterativeTree: public TreeAlgorithm
{
public:
	map<string, vector<Segment> > BuildTrees(map<string, vector<Point> >& pointsets)
	{
		map<Point, int> colors;
		int index = 0;
		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			vector<Point>& points = (*iter).second;
			for (int i = 0; i < (int)points.size(); i++)
				colors[points[i]] = index++;
		}

		map<string, vector<Segment> > trees;
		while (true)
		{
			bool progress = chooseBestEdge(pointsets, trees, colors);
			if (!progress) break;
		}

		return trees;
	}

	bool chooseBestEdge(map<string, vector<Point> >& pointsets, map<string, vector<Segment> >& trees, map<Point, int>& colors)
	{
		double minLen = INF;
		vector<Segment> bestSegs;
		string bestCluster;
		Point bestI, bestJ;

		map<string, double> curMST = computeMST(pointsets, trees);
		//OutputDebugTrees(pointsets, trees, "trees.svg");
		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			vector<Point>& points = (*iter).second;
			string cluster = (*iter).first;

			for (int i = 0; i < (int)points.size(); i++)
				for (int j = i+1; j < (int)points.size(); j++)
				{
					//already connected?
					if (colors[points[i]] == colors[points[j]]) continue;

					vector<Segment> segs = ShortestPath(points, trees, cluster, points[i], points[j]);
					//assert(!segs.empty());
					//already connected?
					if (segs.empty()) continue;
					vector<Segment>& exSegments = trees[cluster];

					int cnt = exSegments.size();
					exSegments.insert(exSegments.begin() + cnt, segs.begin(), segs.end());
					map<string, double> newMST = computeMST(pointsets, trees);

					double len = delta(curMST, newMST) + TreeLength(segs);
					if (len < minLen)
					{
						minLen = len;
						bestSegs = exSegments;
						bestCluster = cluster;
						bestI = points[i];
						bestJ = points[j];
					}

					exSegments.erase(exSegments.begin() + cnt, exSegments.end());
				}
		}

		if (minLen >= INF) return false;
		trees[bestCluster] = bestSegs;

		//merging colors
		int oldColor = colors[bestJ];
		int newColor = colors[bestI];
		vector<Point>& bestPoints = pointsets[bestCluster];
		for (int i = 0; i < (int)bestPoints.size(); i++)
			if (colors[bestPoints[i]] == oldColor) colors[bestPoints[i]] = newColor;

		//OutputTimeInfo("best pair: (%.2lf, %.2lf)  (%.2lf, %.2lf)", bestI.x, bestI.y, bestJ.x, bestJ.y);
		return true;
	}

	double TreeLength(vector<Segment>& segments) const
	{
		double res = 0;
		for (int i = 0; i < (int)segments.size(); i++)
			res += segments[i].length();
		return res;
	}

	map<string, double> computeMST(map<string, vector<Point> >& pointsets, map<string, vector<Segment> >& trees)
	{
		map<string, double> res;

		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			string cluster = (*iter).first;
			double mstLength = PartialMST((*iter).second, trees, cluster);
			res[cluster] = mstLength;
		}

		return res;
	}

	double PartialMST(const vector<Point>& points, map<string, vector<Segment> >& trees, const string& clusterId)
	{
		vector<Segment> obstacles = GetTreeObstacles(trees, clusterId);

		//vis graph computation
		VisibilityGraph visGraph(points, obstacles);

		//indices of VV in the tree
		map<int, int> treeVV = FindSpannedVertices(visGraph, trees[clusterId]);

		//distances between pairs of 
		VVD dist;

		//run dijkstra
		for (int i = 0; i < (int)points.size(); i++)
		{
			pair<VD, VI> dres = RunDijkstra(visGraph, i, treeVV);

			VD tmp;
			for (int j = 0; j < (int)points.size(); j++)
				tmp.push_back(dres.first[j]);

			dist.push_back(tmp);
		}

		//mst construction
		VI mstNodes;
		VVI mstEdges;
		VVD mstDistances;
		for (int i = 0; i < (int)dist.size(); i++)
		{
			mstNodes.push_back(i);
			mstEdges.push_back(VI());
			mstDistances.push_back(VD());

			for (int j = 0; j < (int)dist[i].size(); j++)
			{
				mstEdges[i].push_back(j);
				mstDistances[i].push_back(dist[i][j]);
			}
		}

		return LengthMinimumSpanningTree(mstNodes, mstEdges, mstDistances, 0);
	}

	map<int, int> FindSpannedVertices(VisibilityGraph& visGraph, vector<Segment>& treeSegments)
	{
		//indices of VV in the tree
		map<int, int> treeVV;
		//adding real vertices
		for (int i = 0; i < (int)visGraph.nodes.size(); i++)
		{
			treeVV[i] = i;
		}

		while (true)
		{
			bool progress = false;
			for (int i = 0; i < (int)visGraph.nodes.size(); i++)
			{
				VV node = visGraph.nodes[i];
				for (int j = 0; j < (int)visGraph.edges[node.index].size(); j++)
				{
					VV adj = visGraph.nodes[visGraph.edges[node.index][j]];
					if (treeVV[node.index] == treeVV[adj.index]) continue;

					if (SegmentExists(treeSegments, node.p, adj.p))
					{
						Merge(visGraph, treeVV, node.index, adj.index);
					}
				}
			}

			if (!progress) break;
		}

		return treeVV;
	}

	pair<VD, VI> RunDijkstra(const VisibilityGraph& visGraph, int source, map<int, int>& treeVV)
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
				assert(treeVV.find(i) != treeVV.end() && treeVV.find(adj) != treeVV.end());
				if (treeVV[i] == treeVV[adj]) dist = 0;
				else dist = p1.Distance(p2);

				dDistances[i].push_back(dist);
			}
		}

		return Dijkstra(dNodes, dEdges, dDistances, source);
	}

	vector<Segment> ShortestPath(const vector<Point>& points, map<string, vector<Segment> >& trees, const string& clusterId, const Point& source, const Point& target)
	{
		vector<Segment> obstacles = GetTreeObstacles(trees, clusterId);

		//vis graph computation
		VisibilityGraph visGraph(points, obstacles);

		//indices of VV in the tree
		map<int, int> treeVV = FindSpannedVertices(visGraph, trees[clusterId]);

		vector<VV> path = GetVisibilityPath(visGraph, source, target, treeVV);
		assert(path.size() >= 2);
		assert(path[0].real && path.back().real);
		assert(path[0].p == source && path.back().p == target);

		vector<Segment> result;
		for (int j = 0; j + 1 < (int)path.size(); j++)
		{
			if (treeVV[path[j].index] == treeVV[path[j+1].index]) continue;

			Segment s = Segment(path[j].p, path[j+1].p);
			result.push_back(s);
		}

		return result;
	}

	vector<VisibilityVertex> GetVisibilityPath(const VisibilityGraph& visGraph, const Point& s, const Point& t, map<int, int>& treeVV)
	{
		//find indices
		int sIndex = visGraph.FindIndexOfRealNode(s);
		int tIndex = visGraph.FindIndexOfRealNode(t);
		
		//find path using Dijkstra algorithm
		VI parent = RunDijkstra(visGraph, sIndex, treeVV).second;

		//extract optimal path
		assert(parent[tIndex] != -1);
		vector<VV> res;
		while (tIndex != sIndex)
		{
			res.push_back(visGraph.nodes[tIndex]);
			tIndex = parent[tIndex];
		}

		res.push_back(visGraph.nodes[sIndex]);
		reverse(res.begin(), res.end());

		return res;
	}

	double delta(map<string, double>& curMST, map<string, double>& newMST)
	{
		double res = 0;
		for (auto iter = curMST.begin(); iter != curMST.end(); iter++)
		{
			string cluster = (*iter).first;
			res += newMST[cluster] - curMST[cluster];
		}
		return res;
	}

	bool SegmentExists(const vector<Segment>& treeSegments, const Point& p, const Point& q)
	{
		for (int i = 0; i < (int)treeSegments.size(); i++)
		{
			if (treeSegments[i].first == p && treeSegments[i].second == q) return true;
			if (treeSegments[i].first == q && treeSegments[i].second == p) return true;
		}

		return false;
	}

	void Merge(VisibilityGraph& visGraph, map<int, int>& treeVV, int s, int t)
	{
		int oldColor = treeVV[t];
		int newColor = treeVV[s];
		for (int i = 0; i < (int)visGraph.nodes.size(); i++)
		{
			if (treeVV[i] == oldColor) treeVV[i] = newColor;
		}
	}
};
