#pragma once

#include <vector>
#include <set>
#include <cassert>
using namespace std;

namespace modularity {

class BinaryGraph
{
private:
	BinaryGraph(const BinaryGraph&);
	BinaryGraph& operator = (const BinaryGraph&);

public:
	int n;
	double totalWeight;

	// edges with weights
	vector<vector<int> > links;
	vector<vector<double> > weights;
	// spatial neighbors (contains adjacent nodes)
	vector<set<int> > adj;

	// weighted degree
	vector<double> weightedDegree;
	// weighted self loop
	vector<double> selfLoops;

	BinaryGraph(int n, bool contiguity): n(n), totalWeight(0) 
	{
		links = vector<vector<int> >(n, vector<int>());
		weights = vector<vector<double> >(n, vector<double>());
		if (contiguity)
			adj = vector<set<int> >(n, set<int>());

		weightedDegree = vector<double>(n, 0);
		selfLoops = vector<double>(n, 0);
	}

	void InitWeights()
	{
		for (int i = 0; i < n; i++)
		{
			weightedDegree[i] = getWeightedDegree(i);
			selfLoops[i] = countSelfloops(i);
		}
	}

private:
	// return the number of self loops of the node
	double countSelfloops(int node)	const
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
	double getWeightedDegree(int node) const
	{
		assert (node >= 0 && node < n);

		double res = 0;
		for (int i = 0; i < (int)weights[node].size(); i++)
			res += weights[node][i];
		return res;
	}
};

} // namespace modularity
