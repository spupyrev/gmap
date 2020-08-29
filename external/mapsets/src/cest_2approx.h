#pragma once

#include "graph_algorithms.h"
#include "visibility.h"

//#include "debug_utils.h"

class CEST2Approx: public CESTAlgorithm
{
public:
	map<string, SegmentSet*> BuildTrees(DotGraph& g)
	{
		//DrawSpanningTrees(g);

		VS clusterOrder = GetClusterOrder(g);

		//find optimal spanning tree for each cluster
		vector<Segment> obstacles;
		map<string, SegmentSet*> trees;

		double marginDelta = 0.05;
		double marginCoef = 1.25 + clusterOrder.size() * marginDelta;
		for (int k = 0; k < (int)clusterOrder.size(); k++)
		{
			string clusterId = clusterOrder[k];

			//get obstacles
			vector<Segment> allObstacles = obstacles;
			vector<Segment> boundaryObstacles = GetBoundaryObstacles(g, clusterId, marginCoef);
			allObstacles.insert(allObstacles.end(), boundaryObstacles.begin(), boundaryObstacles.end());

			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentSet* tree = BuildSpanningTree(positions, allObstacles, false);
			//CheckTreeConnected(tree, positions, allObstacles);

			trees[clusterId] = tree;
			for (int i = 0; i < tree->count(); i++)
				obstacles.push_back(tree->get(i));

			marginCoef -= marginDelta;
		}
		return trees;
	}

	/*void DrawSpanningTrees(DotGraph& g)
	{
		auto clusters = g.GetClusters();

		map<string, SegmentSet*> trees;
		for (auto it : clusters)
		{
			string clusterId = it.first;
			cerr << "computing MST for " << clusterId << endl;


			//get obstacles
			vector<Segment> obstacles = GetBoundaryObstacles(g, clusterId, 1.1);
			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentSet* tree = BuildSpanningTree(positions, obstacles);

			trees[clusterId] = tree;
		}

		OutputDebugTrees(g, trees, "test.svg");
	} */

	double GetMSTLength(DotGraph& g)
	{
		map<string, SegmentSet*> trees;
		map<string, vector<DotNode*> > clusters = g.GetClusters();

		for (auto iter = clusters.begin(); iter != clusters.end(); iter++)
		{
			string clusterId = (*iter).first;

			//get obstacles
			vector<Segment> obstacles = GetBoundaryObstacles(g, clusterId, 1.1);
			vector<Point> positions = g.GetClusterPositions(clusterId);
			SegmentSet* tree = BuildSpanningTree(positions, obstacles, false);

			trees[clusterId] = tree;
		}

		return TreeLength(trees);
	}

private:
	VS GetClusterOrder(DotGraph& g)
	{
		map<string, vector<DotNode*> > clusters = g.GetClusters();

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

	SegmentSet* BuildSpanningTree(const vector<Point>& points, const vector<Segment>& obstacles, bool fullVisibility)
	{
		VisibilityGraph visGraph(points, obstacles, fullVisibility);
		//OutputTimeInfo("vis DotGraph constructed");

		VI parent = MinimumSpanningTree(visGraph, 0).second;
		//OutputTimeInfo("mst done");

		//extracting result
		SegmentSet* result = new SegmentSet();
		for (int i = 1; i < (int)points.size(); i++)
		{
			vector<Segment> path;
			ExtractPathToReal(visGraph, parent, 0, i, path);
			//assert(success);

			for (int j = 0; j < (int)path.size(); j++)
			{
				if (!result->contains(path[j]))
					result->append(path[j]);
			}
		}

		return result;
	}

	pair<VD, VI> MinimumSpanningTree(const VisibilityGraph& visGraph, int source)
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
				if (visGraph.nodes[now].real) newDist = Distance(p1, p2);
				else newDist = dist[now] + Distance(p1, p2);

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

	bool ExtractPathToReal(const VisibilityGraph& visGraph, const VI& parent, int s, int t, vector<Segment>& res)
	{
		assert(visGraph.nodes[s].real && visGraph.nodes[t].real);

		//extract optimal path
		if (parent[t] == -1) return false;

		assert(parent[t] != -1);
		while (t != s)
		{
			res.push_back(Segment(visGraph.nodes[t].p, visGraph.nodes[parent[t]].p));
			t = parent[t];
			if (visGraph.nodes[t].real) break;
		}

		return true;
	}

	double Distance(const Point& p1, const Point& p2)
	{
		return p1.Distance(p2);
	}
};