#include "clustering.h"

double ClusterAlgorithm::modularity(ConnectedGraph& g, const VVN& groups) const
{
	double res = 0;

	VN nodes;
	for (int i = 0; i < (int)groups.size(); i++)
		for (int j = 0; j < (int)groups[i].size(); j++)
			nodes.push_back(groups[i][j]);

	double m = 0;
	for (int i = 0; i < (int)nodes.size(); i++)
		for (int j = i+1; j < (int)nodes.size(); j++)
		{
			Node* s = nodes[i];
			Node* t = nodes[j];
			Edge* edge = g.findEdge(s, t);
			if (edge == NULL) continue;

			double w = edge->getWeight();
			m += w;
		}

	for (int i = 0; i < (int)groups.size(); i++)
	{
		for (int j = 0; j < (int)groups[i].size(); j++)
			for (int k = j+1; k < (int)groups[i].size(); k++)
			{
				Node* s = groups[i][j];
				Node* t = groups[i][k];
				Edge* edge = g.findEdge(s, t);
				if (edge == NULL) continue;

				double w = edge->getWeight();
				double deg_s = g.weightedDegree(s);
				double deg_t = g.weightedDegree(t);

				res += (w - deg_s*deg_t/(2.0*m));
			}
	}

	res /= (2.0*m);

	return res;
}

double ClusterAlgorithm::modularity(ConnectedGraph& g, const VI& clusters) const
{
	map<int, VN> tmp;
	for (int i = 0; i < (int)clusters.size(); i++)
	{
		int cl = clusters[i];
		if (cl == -1) continue;

		tmp[cl].push_back(g.nodes[i]);
	}

	VVN res;
	for (map<int, VN>::iterator it = tmp.begin(); it != tmp.end(); it++)
	{
		res.push_back((*it).second);
	}

	return modularity(g, res);
}

void ClusterAlgorithm::cluster(Graph& g, int K)
{
	assert(K >= 1);

	vector<ConnectedGraph> connG = g.getConnectedComponents();
	if ((int)connG.size() > K)
	{
		cerr<<"increasing the number of clusters\n";
		K = (int)connG.size();
	}

	//sort by the number of nodes
	sort(connG.begin(), connG.end());
	int curK = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		int N = g.nodes.size();
		int n = connG[i].nodes.size();

		int k = (n*K) / N;
		if (k < 1) k = 1;
		while (curK + k + ((int)connG.size() - i - 1) > K) k--;
		if (i == (int)connG.size() - 1) k = K - curK;
																				  
		vector<vector<Node*> > clust = cluster(connG[i], k);
		g.assignClusters(clust, curK);
		curK += clust.size();
	}
	assert(curK <= K);
}

void ClusterAlgorithm::cluster(Graph& g)
{
	//guess the number of clusters
	vector<ConnectedGraph> connG = g.getConnectedComponents();
	int K = 0;
	for (int i = 0; i < (int)connG.size(); i++)
	{
		int n = (int)connG[i].nodes.size();
		int dK = (int)sqrt((double)n/2);
		if (dK < 1) dK = 1;
		if (dK > n) dK = n;

		K += dK;
	}

	//cerr<<"guessed number of clusters: "<<K<<"\n";
	cluster(g, K);
}

