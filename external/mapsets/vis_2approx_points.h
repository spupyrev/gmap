#pragma once

#include "test_visibility.h"
#include "graph_algorithms.h"
#include "visibility.h"
#include "time_utils.h"
#include "debug_utils.h"

class Vis2ApproxPoints: public TreePointsAlgorithm
{
public:
	map<string, SegmentTree*> BuildTrees(map<string, vector<Point> >& pointsets)
	{
		OutputTimeInfo("alg started");
		VS clusterOrder = GetClusterOrder(pointsets);
		OutputTimeInfo("order computed");

		//find optimal spanning tree for each cluster
		map<string, SegmentTree*> trees;
		vector<Segment> obstacles;
		for (int k = 0; k < (int)clusterOrder.size(); k++)
		{
			OutputTimeInfo("tree %d started", k);
			string clusterId = clusterOrder[k];

			set<Point> allPoints;
			for (int r = 0; r < (int)clusterOrder.size(); r++)
				if (clusterOrder[r] != clusterId)
				{
					vector<Point> p = pointsets[clusterOrder[r]];
					for (int z = 0; z < (int)p.size(); z++)
						allPoints.insert(p[z]);
				}


			SegmentTree* tree = BuildSpanningTree(pointsets[clusterId], obstacles, allPoints);

			trees[clusterId] = tree;
			OutputDebugTrees(pointsets, trees, "tree" + int2String(k) + ".svg");

			for (int i = 0; i < tree->count(); i++)
				obstacles.push_back(tree->get(i));

			OutputTimeInfo("tree %d done", k);
		}

		OutputTimeInfo("done");
		return trees;
	}

	VS GetClusterOrder(map<string, vector<Point> >& pointsets)
	{
		//order vertices according their MST length
		vector<pair<double, string> > orderedClusters;
		for (auto iter = pointsets.begin(); iter != pointsets.end(); iter++)
		{
			string clusterId = (*iter).first;
			double len = LengthMinimumSpanningTree((*iter).second);
			orderedClusters.push_back(make_pair(len, clusterId));
		}

		sort(orderedClusters.begin(), orderedClusters.end());
		reverse(orderedClusters.begin(), orderedClusters.end());

		VS result;
		for (int i = 0; i < (int)orderedClusters.size(); i++)
			result.push_back(orderedClusters[i].second);

		return result;
	}

	SegmentTree* BuildSpanningTree(const vector<Point>& points, const vector<Segment>& obstacles, set<Point>& allPoints)
	{
		//vis graph computation
		VisibilityGraph visGraph(points, obstacles);
		OutputTimeInfo("vis Graph done");

		VI parent = MinimumSpanningTree(visGraph, 0, allPoints).second;
		OutputTimeInfo("mst done");

		//extracting result
		SegmentTree* result = new SegmentTree();
		for (int i = 1; i < (int)points.size(); i++)
		{
			vector<Segment> path = ExtractPathToReal(visGraph, parent, 0, i);
			for (int j = 0; j < (int)path.size(); j++)
			{
				if (!result->contains(path[j]))
					result->append(path[j]);
			}
		}

		return result;
	}

	pair<VD, VI> MinimumSpanningTree(const VisibilityGraph& visGraph, int source, set<Point>& allPoints)
	{
		VI used = VI(visGraph.nodes.size(), 0);
		VI parent = VI(visGraph.nodes.size(), -1);
		VD dist(visGraph.nodes.size(), INF);
		assert(visGraph.nodes[source].real);
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

			//update neighbors
			for (int i = 0; i < (int)visGraph.edges[now].size(); i++)
			{
				int next = visGraph.edges[now][i];
				if (used[next]) continue;

				Point p1 = visGraph.nodes[now].p;
				Point p2 = visGraph.nodes[next].p;

				double newDist;
				if (visGraph.nodes[now].real) newDist = Distance(p1, p2, allPoints);
				else newDist = dist[now] + Distance(p1, p2, allPoints);

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

	vector<Segment> ExtractPathToReal(const VisibilityGraph& visGraph, const VI& parent, int s, int t)
	{
		assert(visGraph.nodes[s].real && visGraph.nodes[t].real);

		//extract optimal path
		assert(parent[t] != -1);
		vector<Segment> res;
		while (t != s)
		{
			res.push_back(Segment(visGraph.nodes[t].p, visGraph.nodes[parent[t]].p));
			t = parent[t];
			if (visGraph.nodes[t].real) break;
		}

		return res;
	}

	double Distance(const Point& p1, const Point& p2, set<Point>& points)
	{
		double dMin = INF;
		for (auto iter = points.begin(); iter != points.end(); iter++)
		{
			Point q = (*iter);
			double d = ProjectionDistance(p1, p2, q);
			if (d == -1) continue;
			dMin = min(dMin, d);
		}

		double delta = 1000.0/dMin;
		if (dMin > 100) delta = 0;
		if (dMin < EPS) delta = 10000;

		//delta = 0;
		return (p1.Distance(p2) + delta);
	}

	double ProjectionDistance(const Point& segmentStart, const Point& segmentEnd, const Point& point)
	{
        Point bc = segmentEnd - segmentStart;
        Point ba = point - segmentStart;
        double c1, c2;
		if ((c1 = Point::DotProduct(bc, ba)) <= 0.0 - EPS) 
			return -1;

		if ((c2 = Point::DotProduct(bc, bc)) <= c1 - EPS) 
			return -1;

        double parameter = c1 / c2;
		return (ba - bc*parameter).Length();
	}
};