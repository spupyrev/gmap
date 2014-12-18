#include "clustering.h"

#include "infomap/infohiermap.h"

using namespace infomap;

void InfoMap::cluster(DotGraph& g)
{
	vector<ConnectedDotGraph> connG = g.getConnectedComponents();

	int curK = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		vector<vector<DotNode*> > clust = runInfomap(connG[i], 10, 1.0);
		g.AssignClusters(clust, curK);
		curK += clust.size();
	}
}

void InfoMap::cluster(DotGraph& g, int K)
{
	cluster(g);
}
