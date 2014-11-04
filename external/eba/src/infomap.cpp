#include "clustering.h"

#include "infomap/infohiermap.h"

void InfoMap::cluster(Graph& g)
{
	vector<ConnectedGraph> connG = g.getConnectedComponents();

	int curK = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		vector<vector<Node*> > clust = runInfomap(connG[i], 10, 1.0);
		g.assignClusters(clust, curK);
		curK += clust.size();
	}
}

void InfoMap::cluster(Graph& g, int K)
{
	cluster(g);
}
