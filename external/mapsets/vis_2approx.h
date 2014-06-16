#pragma once

#include "graph_algorithms.h"
#include "visibility.h"
#include "time_utils.h"
#include "debug_utils.h"

class Vis2Approx: public TreeAlgorithm
{
public:
	map<string, SegmentTree*> BuildTrees(Graph& g)
	{
		VS clusterOrder = GetClusterOrder(g);
		//OutputTimeInfo("order computed");

		//find optimal spanning tree for each cluster
		vector<Segment> obstacles;
		map<string, SegmentTree*> trees;

		double marginDelta = 0.05;
		double marginCoef = 1.25 + clusterOrder.size() * marginDelta;
		for (int k = 0; k < (int)clusterOrder.size(); k++)
		{
			//OutputTimeInfo("tree %d started", k);
			string clusterId = clusterOrder[k];

			//get obstacles
			vector<Segment> allObstacles = obstacles;
			vector<Segment> boundaryObstacles = GetBoundaryObstacles(g, clusterId, marginCoef);
			allObstacles.insert(allObstacles.end(), boundaryObstacles.begin(), boundaryObstacles.end());

			set<Point> allPoints;
			/*for (int r = 0; r < (int)clusterOrder.size(); r++)
				if (clusterOrder[r] != clusterId)
				{
					vector<Point> p = g.GetClusterPositions(clusterOrder[r]);
					for (int z = 0; z < (int)p.size(); z++)
						allPoints.insert(p[z]);
				}*/


			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentTree* tree = BuildSpanningTree(positions, allObstacles, allPoints);
			CheckTreeConnected(tree, positions, allObstacles);

			trees[clusterId] = tree;
			for (int i = 0; i < tree->count(); i++)
				obstacles.push_back(tree->get(i));

			marginCoef -= marginDelta;
			//OutputTimeInfo("tree %d done", k);
		}
		return trees;
	}

	double GetMSTLength(Graph& g)
	{
		map<string, SegmentTree*> trees;
		map<string, vector<Node*> > clusters = g.GetClusters();

		for (auto iter = clusters.begin(); iter != clusters.end(); iter++)
		{
			string clusterId = (*iter).first;

			//get obstacles
			vector<Segment> obstacles = GetBoundaryObstacles(g, clusterId, 1.1);
			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentTree* tree = BuildSpanningTree(positions, obstacles);

			trees[clusterId] = tree;
		}

		return TreeLength(trees);
	}

	double Get2ApproxLength(Graph& g)
	{
		VS clusterOrder = GetClusterOrder(g);

		double res = 0;
		int k = (int)clusterOrder.size();
		for (int i = 0; i < k; i++)
		{
			string clusterId = clusterOrder[i];

			//get obstacles
			vector<Segment> obstacles = GetBoundaryObstacles(g, clusterId, 1.1);
			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentTree* tree = BuildSpanningTree(positions, obstacles);

			res += tree->Length() * (2*k - 2*i + 1);
		}

		return res;
	}

private:
	VS GetClusterOrder(Graph& g)
	{
		map<string, vector<Node*> > clusters = g.GetClusters();

		//order vertices according their MST length
		vector<pair<double, string> > orderedClusters;
		for (auto iter = clusters.begin(); iter != clusters.end(); iter++)
		{
			string clusterId = (*iter).first;
			double len = LengthMinimumSpanningTree(g.GetClusterPositions(clusterId));
			orderedClusters.push_back(make_pair(len, clusterId));
		}

		sort(orderedClusters.begin(), orderedClusters.end());
		//reverse(orderedClusters.begin(), orderedClusters.end());

		VS result;
		for (int i = 0; i < (int)orderedClusters.size(); i++)
			result.push_back(orderedClusters[i].second);

		return result;
	}

	SegmentTree* BuildSpanningTree(const vector<Point>& points, const vector<Segment>& obstacles)
	{
		set<Point> allPoints;
		return BuildSpanningTree(points, obstacles, allPoints);
	}

	SegmentTree* BuildSpanningTree(const vector<Point>& points, const vector<Segment>& obstacles, set<Point>& allPoints)
	{
		//vis graph computation
		VisibilityGraph visGraph(points, obstacles);
		//OutputTimeInfo("vis Graph constructed");

		VI parent = MinimumSpanningTree(visGraph, 0, allPoints).second;
		//OutputTimeInfo("mst done");

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
		return p1.Distance(p2);

		double dMin = INF;
		for (auto iter = points.begin(); iter != points.end(); iter++)
		{
			Point q = (*iter);
			double d = ProjectionDistance(p1, p2, q);
			if (d == -1) continue;
			dMin = min(dMin, d);
		}

		double delta = 100.0/dMin;
		if (dMin > 40) delta = 0;
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