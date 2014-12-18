#include "clustering.h"

#include <map>
#include <algorithm>
#include <iostream>
#include <cassert>

ClusteringInfo::ClusteringInfo(ConnectedDotGraph* g, const VVN& groups)
{
	this->g = g;

	//init cluster
	cluster = VI(g->maxNodeIndex(), -1);
	clusterCount = (int)groups.size();
	for (int i = 0; i < (int)groups.size(); i++)
		for (int j = 0; j < (int)groups[i].size(); j++)
			cluster[groups[i][j]->index] = i;

	//modularity
	modularity = getModularitySlow();

	//cache
	m2 = 0;
	for (int i = 0; i < (int)g->nodes.size(); i++)
		m2 += g->weightedDegree(g->nodes[i]);

	sumTot = VD(clusterCount, 0);
	for (int i = 0; i < (int)groups.size(); i++)
		for (int j = 0; j < (int)groups[i].size(); j++)
			sumTot[i] += g->weightedDegree(groups[i][j]);

	sumIn = VD(clusterCount, 0);
	for (int i = 0; i < (int)g->edges.size(); i++)
	{
		DotEdge* e = g->edges[i];
		DotNode* u = g->findNodeById(e->s);
		DotNode* v = g->findNodeById(e->t);

		if (cluster[u->index] == cluster[v->index])
			sumIn[cluster[u->index]] += e->getWeight();
	}
}

VVN ClusteringInfo::getGroups() const
{
	map<int, VN> groups;

	for (int i = 0; i < (int)g->nodes.size(); i++)
	{
		DotNode* v = g->nodes[i];
		if (cluster[v->index] != -1)
			groups[cluster[v->index]].push_back(v);
	}

	vector<vector<DotNode*> > groupsRes;
	for (auto iter = groups.begin(); iter != groups.end(); iter++)
		groupsRes.push_back((*iter).second);

	return groupsRes;
}

void ClusteringInfo::moveVertex(const DotNode* node, int newCluster)
{
	int curCluster = cluster[node->index];
	assert(curCluster != -1 && curCluster != newCluster);

	double wd = g->weightedDegree(node);

	//move node from the current to an isolated community
	double ki = wd;
	double ki_in = computeKiIn(node, curCluster);
	double sum_in = sumIn[curCluster] - ki_in;
	double sum_tot = sumTot[curCluster] - wd;

	double delta = ((sum_in+2*ki_in)/m2 - Sqr2((sum_tot+ki)/m2)) - (sum_in/m2 - Sqr2(sum_tot/m2) - Sqr2(ki/m2));

	//updating cache
	sumTot[curCluster] -= wd;
	sumIn[curCluster] -= ki_in;

	//move node from the isolated to the new community
	ki = wd;
	ki_in = computeKiIn(node, newCluster);
	sum_in = sumIn[newCluster];
	sum_tot = sumTot[newCluster];

	double delta2 = ((sum_in+2*ki_in)/m2 - Sqr2((sum_tot+ki)/m2)) - (sum_in/m2 - Sqr2(sum_tot/m2) - Sqr2(ki/m2));

	//updating cache
	sumTot[newCluster] += wd;
	sumIn[newCluster] += ki_in;

	//actual move
	cluster[node->index] = newCluster;
	modularity += delta2 - delta;
	//checkConsistency();
}

void ClusteringInfo::checkConsistency()
{
	assert(Abs(modularity - getModularitySlow()) < 1e-4);

	int maxGroup = 0;
	map<int, double> sumTot2;
	map<int, double> sumIn2;

	for (int i = 0; i < (int)g->nodes.size(); i++)
	{
		DotNode* v = g->nodes[i];
		sumTot2[cluster[v->index]] += g->weightedDegree(v);
		maxGroup = max(maxGroup, cluster[v->index]);
	}

	for (int i = 0; i < (int)g->edges.size(); i++)
	{
		DotEdge* e = g->edges[i];
		DotNode* u = g->findNodeById(e->s);
		DotNode* v = g->findNodeById(e->t);

		if (cluster[u->index] == cluster[v->index])
			sumIn2[cluster[u->index]] += e->getWeight();
	}

	for (int i = 0; i <= maxGroup; i++)
	{
		assert(Abs(sumTot[i] - sumTot2[i]) < 1e-4);
		assert(Abs(sumIn[i] - sumIn2[i]) < 1e-4);
	}
}

double ClusteringInfo::computeKiIn(const DotNode* node, int clusterId) const
{
	double ki_in = 0;
	vector<pair<DotNode*, DotEdge*> > adj = g->getAdj(node);
	for (int i = 0; i < (int)adj.size(); i++)
	{
		DotNode* u = adj[i].first;
		if (u == node) continue;
		if (cluster[u->index] != clusterId) continue;
		ki_in += adj[i].second->getWeight();
	}

	return ki_in;
}

double ClusteringInfo::getModularitySlow() const
{
	double m = 0;
	for (int i = 0; i < (int)g->nodes.size(); i++)
		m += g->weightedDegree(g->nodes[i]) / 2.0;

	double res = 0;
	for (int i = 0; i < (int)g->nodes.size(); i++)
		for (int j = 0; j < (int)g->nodes.size(); j++)
		{
			DotNode* s = g->nodes[i];
			DotNode* t = g->nodes[j];
			DotEdge* edge = g->findEdge(s, t);

			int cluster_s = cluster[s->index];
			int cluster_t = cluster[t->index];
			assert(cluster_s != -1 && cluster_t != -1);
			if (cluster_s != cluster_t) continue;

			double w = (edge != NULL ? edge->getWeight() : 0);
			double deg_t = g->weightedDegree(t);
			double deg_s = g->weightedDegree(s);

			res += (w - deg_s * deg_t / (2.0 * m));
		}

	res /= (2.0 * m);

	return res;
}


void ClusterAlgorithm::cluster(DotGraph& g, int K)
{
	assert(K >= 1);

	vector<ConnectedDotGraph> connG = g.getConnectedComponents();
	if ((int)connG.size() > K)
	{
		cerr << "increasing the number of clusters\n";
		K = (int)connG.size();
	}

	//sort by the number of nodes
	sort(connG.begin(), connG.end());
	int curK = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		int N = g.nodes.size();
		int n = connG[i].nodes.size();

		int k = (n * K) / N;
		if (k < 1) k = 1;
		while (curK + k + ((int)connG.size() - i - 1) > K) k--;
		if (i == (int)connG.size() - 1) k = K - curK;

		vector<vector<DotNode*> > clust = cluster(connG[i], k);
		g.AssignClusters(clust, curK);
		curK += clust.size();
	}
	assert(curK <= K);
}

void ClusterAlgorithm::cluster(DotGraph& g)
{
	//guess the number of clusters
	vector<ConnectedDotGraph> connG = g.getConnectedComponents();
	int K = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		int n = (int)connG[i].nodes.size();
		int dK = (int)(sqrt((double)n / 2) + 0.5);
		if (dK < 1) dK = 1;
		if (dK > n) dK = n;

		K += dK;
	}

	//cerr<<"guessed number of clusters: "<<K<<"\n";
	cluster(g, K);
}

