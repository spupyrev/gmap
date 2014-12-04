#include "clustering.h"

#include "modularity/discover_community.h"

using namespace modularity;

void Modularity::cluster(DotGraph& g)
{
	vector<ConnectedDotGraph> connG = g.getConnectedComponents();

	int curK = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		vector<vector<DotNode*> > clust = runModularity(connG[i]);
		g.assignClusters(clust, curK);
		curK += clust.size();
	}
}

void Modularity::cluster(DotGraph& g, int K)
{
	cluster(g);
}
