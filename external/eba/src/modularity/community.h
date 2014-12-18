#pragma once

#include "binary_graph.h"
#include "cluster.h"

#include <vector>
#include <algorithm>
#include <map>
#include <cassert>
using namespace std;

namespace modularity {

class Community
{
private:
	BinaryGraph* g;
	bool contiguity;
	vector<int> n2c;
	vector<double> in;
	vector<double> tot;
	vector<int> totCount;

	Community(const Community&);
	Community& operator = (const Community&);

public:
	Community(BinaryGraph* g, bool contiguity) : g(g), contiguity(contiguity)
	{
		n2c = vector<int>(g->n);
		in = vector<double>(g->n);
		tot = vector<double>(g->n);
		totCount = vector<int>(g->n);

		for (int i = 0; i < g->n; i++)
		{
			n2c[i] = i;
			in[i] = g->selfLoops[i];
			tot[i] = g->weightedDegree[i];
			totCount[i] = 1;
		}
	}

	~Community()
	{
		delete g;
	}

	void insert(int node, int comm, double dnodecomm)
	{
		tot[comm] += g->weightedDegree[node];
		totCount[comm] += 1;
		in[comm] += 2 * dnodecomm + g->selfLoops[node];
		n2c[node] = comm;
	}

	void remove(int node, int comm, double dnodecomm)
	{
		assert (node >= 0 && node < g->n);

		tot[comm] -= g->weightedDegree[node];
		totCount[comm] -= 1;
		in[comm] -= 2 * dnodecomm + g->selfLoops[node];
		n2c[node] = -1;
	}

	double gain_modularity(int node, int comm, double dnodecomm)
	{
		assert (node >= 0 && node < g->n);

		double totc = tot[comm];
		double degc = g->weightedDegree[node];
		double m2 = g->totalWeight;
		double dnc = dnodecomm;

		return (dnc - totc * degc / m2);
	}

	double modularity() const;

	// computation of all neighboring communities of current node
	map<int, double> neigh_comm(int node) const;

	double one_level();

	BinaryGraph* prepareBinaryGraph() const;

	Cluster* prepareCluster(const Cluster* rootCluster) const;
};

} // namespace modularity
