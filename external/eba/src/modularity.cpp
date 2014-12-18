#include "clustering.h"

#include "modularity/discover_community.h"

void Modularity::cluster(DotGraph& g)
{
	vector<vector<DotNode*> > clust = modularity::runModularity(g, contigous);
	g.AssignClusters(clust, 0);
}

void Modularity::cluster(DotGraph& g, int K)
{
	cluster(g);
}
