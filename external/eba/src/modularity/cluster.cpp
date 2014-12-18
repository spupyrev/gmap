#include "cluster.h"

#include <vector>
#include <set>
#include <map>
#include <cassert>
using namespace std;

namespace modularity {

vector<int> Cluster::extractAllVertices() const
{
	if (containsVertices())
	{
		vector<int>	subVertexes;
		for (int i = 0; i < (int)vertexes.size(); i++)
			subVertexes.push_back(vertexes[i]);
		return subVertexes;
	}
	else
	{
		vector<int>	subVertexes;
		for (int i = 0; i < (int)subClusters.size(); i++)
		{
			vector<int> vall = subClusters[i]->extractAllVertices();
			for (int j = 0 ; j < (int)vall.size(); j++)
				subVertexes.push_back(vall[j]);
		}

		return subVertexes;
	}
}

} // namespace modularity
