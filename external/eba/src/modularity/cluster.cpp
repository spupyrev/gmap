#include "cluster.h"

#include <vector>
#include <set>
#include <map>
#include <cassert>
using namespace std;

namespace modularity {

int Cluster::getSize() const
{
	if (containsVertices())
	{
		return (int)getVertexes().size();
	}
	else
	{
		return (int)getSubClusters().size();
	}
}

BinaryGraph Cluster::constructSubBinaryGraph(const vector<int>& indexes) const
{
	int* old2New = new int[binaryGraph.n];
	for (int i=0;i<binaryGraph.n;i++)
		old2New[i] = -1;

	int c = 0;
	for (int i=0;i<(int)indexes.size();i++)
		old2New[indexes[i]] = c++;

	BinaryGraph graph;

	graph.n = (int)indexes.size();
	graph.totalWeight = 0;
	graph.links = vector<vector<int> >(graph.n, vector<int>());
	graph.weights = vector<vector<double> >(graph.n, vector<double>());

	for (int i = 0; i < (int)indexes.size(); i++)
		if (old2New[i] != -1)
		{
			int oldIndex = indexes[i];

			vector<int> l;
			for (int j = 0; j < (int)binaryGraph.links[oldIndex].size(); j++)
				if (old2New[binaryGraph.links[oldIndex][j]] != -1)
					l.push_back(j);

			graph.links[i] = vector<int>(l.size(), 0);
			graph.weights[i] = vector<double>(l.size(), 0);
			for (int j = 0; j < (int)l.size(); j++)
			{
				graph.links[i][j] = old2New[binaryGraph.links[oldIndex][l[j]]];
				graph.weights[i][j] = binaryGraph.weights[oldIndex][l[j]];
				graph.totalWeight += graph.weights[i][j];
			}
		}

	delete[] old2New;
	return graph;
}

vector<ModularityVertex*> Cluster::extractAllVertices() const
{
	if (containsVertices())
	{
		vector<ModularityVertex*>	subVertexes;
		for (int i = 0; i < (int)vertexes.size(); i++)
			subVertexes.push_back(vertexes[i]);
		return subVertexes;
	}
	else
	{
		vector<ModularityVertex*>	subVertexes;
		for (int i = 0; i < (int)subClusters.size(); i++)
		{
			vector<ModularityVertex*> vall = subClusters[i]->extractAllVertices();
			for (int j = 0 ; j < (int)vall.size(); j++)
				subVertexes.push_back(vall[j]);
		}

		return subVertexes;
	}
}

} // namespace modularity
