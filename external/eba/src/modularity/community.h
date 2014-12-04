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
	BinaryGraph g;
	int size;
	vector<int> n2c;
	vector<double> in;
	vector<double> tot;
	vector<int> totCount;
	int nb_pass;
	double minModularity;

public:
	Community(BinaryGraph graph, int nbp, double minModularity) : g(graph), nb_pass(nbp), minModularity(minModularity)
	{
		size = graph.n;

		n2c = vector<int>(size);
		in = vector<double>(size);
		tot = vector<double>(size);
		totCount = vector<int>(size);

		for (int i = 0; i < size; i++)
		{
			n2c[i] = i;
			in[i] = graph.countSelfloops(i);
			tot[i] = graph.getWeightedDegree(i);
			totCount[i] = 1;
		}
	}

	inline void insert(int node, int comm, double dnodecomm)
	{
		tot[comm] += g.getWeightedDegree(node);
		totCount[comm] += 1;
		in[comm] += 2 * dnodecomm + g.countSelfloops(node);
		n2c[node] = comm;
	}

	inline void remove(int node, int comm, double dnodecomm)
	{
		assert (node >= 0 && node < size);

		tot[comm] -= g.getWeightedDegree(node);
		totCount[comm] -= 1;
		in[comm] -= 2 * dnodecomm + g.countSelfloops(node);
		n2c[node] = -1;
	}

	inline double gainModularity(int node, int comm, double dnodecomm)
	{
		assert (node >= 0 && node < size);

		double totc = tot[comm];
		double degc = g.getWeightedDegree(node);
		double m2 = g.totalWeight;
		double dnc = dnodecomm;

		return (dnc - totc * degc / m2);
	}

	double modularity() const;

	// computation of all neighboring communities of current node
	map<int, double> neigh_comm(int node) const;

	double one_level(const Cluster* rootCluster);

	BinaryGraph partition2graph_binary() const;

	Cluster* prepareCluster(const Cluster* rootCluster, const BinaryGraph& binaryGraph) const;
};

} // namespace modularity
