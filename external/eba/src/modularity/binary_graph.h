#pragma once

#include "modularity_graph.h"

#include <vector>
#include <cassert>
using namespace std;

namespace modularity {

class BinaryGraph
{
public:
	int n;
	double totalWeight;

	vector<vector<int> > links;
	vector<vector<double> > weights;

	BinaryGraph() {}

	BinaryGraph(ModularityGraph& graph)
	{
		// read number of nodes on 4 bytes
		n = graph.getSize();
		totalWeight = 0;

		// read links: 4 bytes for each link (each link is counted twice)
		links = vector<vector<int> >(n, vector<int>());
		weights = vector<vector<double> >(n, vector<double>());
		for (int i = 0; i < n; i++)
		{
			int edgeSize = graph.getVertex(i)->getEdgesSize();

			links[i] = vector<int>(edgeSize);
			weights[i] = vector<double>(edgeSize);
			for (int j = 0; j < edgeSize; j++)
			{
				links[i][j] = graph.getVertex(i)->getEdge(j);
				weights[i][j] = graph.getVertex(i)->getWeight(j);
				totalWeight += weights[i][j];
			}
		}
	}

	// return the number of self loops of the node
	inline double countSelfloops(int node)
	{
		assert (node >= 0 && node < n);

		for (int i = 0; i < (int)links[node].size(); i++)
		{
			if (links[node][i] == node)
				return weights[node][i];
		}
		return 0;
	}

	// return the weighted degree of the node
	inline double getWeightedDegree(int node)
	{
		assert (node >= 0 && node < n);

		double res = 0;
		for (int i = 0; i < (int)weights[node].size(); i++)
			res += weights[node][i];
		return res;
	}
};

} // namespace modularity
