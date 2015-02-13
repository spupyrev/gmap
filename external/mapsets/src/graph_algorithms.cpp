#include "graph_algorithms.h"
#include <queue>
#include <set>
#include <cassert>

pair<VD, VI> GenericDijkstraMSTAlgorithm(const VI& nodes, const VVI& edges, const VVD& distances, int source, bool isDijkstra)
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

		//update neighbors
		for (int i = 0; i < (int)edges[now].size(); i++)
		{
			int next = edges[now][i];
			if (used[next]) continue;

			double newDist = (isDijkstra ? dist[now] + distances[now][i] : distances[now][i]);

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

//returns pair<distances_to_source, parents>
pair<VD, VI> Dijkstra(const VI& nodes, const VVI& edges, const VVD& distances, int source)
{
	return GenericDijkstraMSTAlgorithm(nodes, edges, distances, source, true);
}

//returns pair<distances_to_source, parents>
pair<VD, VI> MinimumSpanningTree(const VI& nodes, const VVI& edges, const VVD& distances, int source)
{
	return GenericDijkstraMSTAlgorithm(nodes, edges, distances, source, false);
}

//returns length of the tree
double LengthMinimumSpanningTree(const VI& nodes, const VVI& edges, const VVD& distances, int source)
{
	VI parent = MinimumSpanningTree(nodes, edges, distances, source).second;
	double sum = 0;
	for (int i = 0; i < (int)parent.size(); i++)
	{
		if (i == source) {assert(parent[i] == -1); continue;}
		else assert(parent[i] != -1);

		int indexOfParent = -1;
		for (int j = 0; j < (int)edges[i].size(); j++)
			if (edges[i][j] == parent[i]) {indexOfParent = j; break;}
		assert(indexOfParent != -1);

		sum += distances[i][indexOfParent];
	}

	return sum;
}

//length of minimum spanning tree on a pointset
double LengthMinimumSpanningTree(const vector<Point>& p)
{
	//mst construction
	VI mstNodes;
	VVI mstEdges;
	VVD mstDistances;
	for (int i = 0; i < (int)p.size(); i++)
	{
		mstNodes.push_back(i);
		mstEdges.push_back(VI());
		mstDistances.push_back(VD());

		for (int j = 0; j < (int)p.size(); j++)
			if (i != j)
			{
				mstEdges[i].push_back(j);
				mstDistances[i].push_back(p[i].Distance(p[j]));
			}
	}

	return LengthMinimumSpanningTree(mstNodes, mstEdges, mstDistances, 0);
}
