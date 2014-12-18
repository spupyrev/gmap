#include "community.h"

#include <set>

namespace modularity {

double Community::modularity() const
{
	double q = 0.;
	double m2 = g->totalWeight;

	for (int i = 0; i < g->n; i++)
	{
		if (tot[i] > 0)
			q += in[i] / m2 - (tot[i] / m2) * (tot[i] / m2);
	}

	return q;
}

// computation of all neighboring communities of current node
map<int, double> Community::neigh_comm(int node) const
{
	set<int> adj_comm;
	if (contiguity)
	{
		for (auto adj_neigh : g->adj[node])
		{
			if (adj_neigh == node) continue;

			int neigh_comm = n2c[adj_neigh];
			adj_comm.insert(neigh_comm);
		}
	}

	map<int, double> res;
	int comm = n2c[node];
	res[comm] = 0;
	for (int i = 0; i < (int)g->links[node].size(); i++)
	{
		int neigh = g->links[node][i];
		if (neigh == node) continue;

		int neigh_comm = n2c[neigh];
		if (contiguity && neigh_comm != comm && !adj_comm.count(neigh_comm)) continue;

		double neigh_weight = g->weights[node][i];
		res[neigh_comm] += neigh_weight;
	}


	return res;
}

double Community::one_level()
{
	bool improvement = false;
	int nb_pass_done = 0;
	double new_mod = modularity();
	double cur_mod = new_mod;

	// repeat while 
	//   there is an improvement of modularity
	//   or there is an improvement of modularity greater than a given epsilon 
	//   or a predefined number of pass have been done

	do
	{
		cur_mod = new_mod;
		improvement = false;
		nb_pass_done++;

		// for each node: remove the node from its community and insert it in the best community
		for (int node_tmp = 0; node_tmp < g->n; node_tmp++)
		{
			int node = node_tmp;
			int node_comm = n2c[node];

			// computation of all neighboring communities of current node
			map<int, double> ncomm = neigh_comm(node);

			// remove node from its current community
			remove(node, node_comm, ncomm[node_comm]);

			// compute the nearest community for node
			// default choice for future insertion is the former community
			int best_comm = node_comm;
			double best_nblinks = 0;
			double best_increase = 0;
			for (auto iter = ncomm.begin(); iter != ncomm.end(); iter++)
			{
				int new_comm = (*iter).first;
				double increase = gain_modularity(node, new_comm, ncomm[new_comm]);
				if (increase > best_increase + 1e-8)
				{
					best_comm = new_comm;
					best_nblinks = ncomm[new_comm];
					best_increase = increase;
				}
			}

			insert(node, best_comm, best_nblinks);

			if (best_comm != node_comm)
				improvement = true;
		}

		new_mod = modularity();
		//cerr << "pass number " << nb_pass_done << " of " << nb_pass << " : " << new_mod << " " << cur_mod << endl;
	} 
	while (improvement && new_mod - cur_mod > 1e-6);

	return new_mod;
}

BinaryGraph* Community::prepareBinaryGraph() const
{
	map<int, vector<int> > comm;
	map<int, int> comm2Order;
	vector<int> order;
	for (int i = 0; i < g->n; i++)
	{
		if (!comm.count(n2c[i]))
		{
			comm[n2c[i]] = vector<int>();
			order.push_back(n2c[i]);
			comm2Order[n2c[i]] = (int)order.size() - 1;
		}

		comm[n2c[i]].push_back(i);
	}

	// unweigthed to weighted
	BinaryGraph* g2 = new BinaryGraph((int)order.size(), contiguity);
	// initialize edges
	for (int ci = 0; ci < (int)order.size(); ci++)
	{
		int cIndex = order[ci];
		map<int, double> m;

		for (int i = 0; i < (int)comm[cIndex].size(); i++)
		{
			int vIndex = comm[cIndex][i];
			for (int i = 0; i < (int)g->links[vIndex].size(); i++)
			{
				int neigh = g->links[vIndex][i];
				int neigh_comm = comm2Order[n2c[neigh]];
				double neigh_weight = g->weights[vIndex][i];

				m[neigh_comm] += neigh_weight;
			}

			if (contiguity)
			{
				for (auto adj_neigh : g->adj[vIndex])
				{
					int neigh_comm = comm2Order[n2c[adj_neigh]];
					g2->adj[ci].insert(neigh_comm);
				}
			}
		}

		for (auto it : m)
		{
			int neigh_comm = it.first;
			g2->totalWeight += m[neigh_comm];
			g2->links[ci].push_back(neigh_comm);
			g2->weights[ci].push_back(m[neigh_comm]);
		}
	}

	g2->InitWeights();

	return g2;
}

Cluster* Community::prepareCluster(const Cluster* rootCluster) const
{
	Cluster* cluster = new Cluster();
	map<int, vector<int> > comm;
	vector<int> order;
	for (int i = 0; i < g->n; i++)
	{
		if (!comm.count(n2c[i]))
		{
			comm[n2c[i]] = vector<int>();
			order.push_back(n2c[i]);
		}

		comm[n2c[i]].push_back(i);
	}

	for (int ci = 0; ci < (int)order.size(); ci++)
	{
		int cIndex = order[ci];
		Cluster* subCluster = new Cluster();
		for (int i = 0; i < (int)comm[cIndex].size(); i++)
		{
			int vIndex = comm[cIndex][i];
			if (rootCluster->containsVertices())
				subCluster->addVertex(rootCluster->getVertexes()[vIndex]);
			else
				subCluster->addSubCluster(rootCluster->getSubClusters()[vIndex]);
		}

		cluster->addSubCluster(subCluster);
	}

	return cluster;
}

} // namespace modularity
