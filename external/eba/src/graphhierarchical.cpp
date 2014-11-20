#include "common/common.h"
#include "clustering.h"

double GraphHierarchical::computeAverageLength(ConnectedDotGraph& g, const VN& v1, const VN& v2)
{
	VD avg;
	for (int i = 0; i < (int)v1.size(); i++)
		for (int j = 0; j < (int)v2.size(); j++)
		{
			double len = g.getShortestPath(v1[i], v2[j], true);
			avg.push_back(len);
		}

	//return min(v1.size(), v2.size()) * AverageValue(avg);
	return Maximum(avg);
}

vector<vector<DotNode*> > GraphHierarchical::cluster(ConnectedDotGraph& g, int K)
{
	assert(K >= 1);

	VVN res;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		VN t;
		t.push_back(g.nodes[i]);
		res.push_back(t);
	}

	while ((int)res.size() > K)
	{
		//find closest
		int bi = -1, bj = -1;
		double bValue = -1;
		for (int i = 0; i < (int)res.size(); i++)
			for (int j = i + 1; j < (int)res.size(); j++)
			{
				double avgLen = computeAverageLength(g, res[i], res[j]);
				if (bValue == -1 || bValue > avgLen)
				{
					bValue = avgLen;
					bi = i;
					bj = j;
				}
			}

		//merge
		if (bi == -1) break;
		res[bi].insert(res[bi].end(), res[bj].begin(), res[bj].end());
		res.erase(res.begin() + bj);
	}

	return res;
}

