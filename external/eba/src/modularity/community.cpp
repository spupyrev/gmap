#include "community.h"

namespace modularity {

double Community::modularity() const
{
	double q = 0.;
	double m2 = g.totalWeight;

	for (int i = 0; i < size; i++)
	{
		if (tot[i] > 0)
			q += in[i] / m2 - (tot[i] / m2) * (tot[i] / m2);
	}

	return q;
}

// computation of all neighboring communities of current node
map<int, double> Community::neigh_comm(int node) const
{
	map<int, double> res;

	res[n2c[node]] = 0;
	for (int i = 0; i < (int)g.links[node].size(); i++)
	{
		int neigh = g.links[node][i];
		int neigh_comm = n2c[neigh];
		double neigh_weight = g.weights[node][i];

		if (neigh != node)
		{
			res[neigh_comm] += neigh_weight;
		}
	}

	return res;
}

double Community::one_level(const Cluster* rootCluster)
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
		for (int node_tmp = 0; node_tmp < size; node_tmp++)
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
			double best_nblinks = 0;//ncomm.find(node_comm)->second;
			double best_increase = 0.;//modularity_gain(node, best_comm, best_nblinks);
			for (auto iter = ncomm.begin(); iter != ncomm.end(); iter++)
			{
				int it = (*iter).first;
				double increase = gainModularity(node, it, ncomm[it]);
				if (increase > best_increase)
				{
					best_comm = it;
					best_nblinks = ncomm[it];
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
	while (improvement && new_mod - cur_mod > minModularity && nb_pass_done != nb_pass);

	return new_mod;
}

BinaryGraph Community::partition2graph_binary() const
{
	map<int, vector<int> > comm;
	map<int, int> comm2Order;
	vector<int> order;
	for (int i = 0; i < size; i++)
	{
		if (comm.find(n2c[i])==comm.end())
		{
			comm[n2c[i]] = vector<int>();
			order.push_back(n2c[i]);
			comm2Order[n2c[i]] = (int)order.size() - 1;
		}

		comm[n2c[i]].push_back(i);
	}

	// unweigthed to weighted
	BinaryGraph g2;
	g2.n = (int)order.size();
	g2.totalWeight = 0;
	g2.links = vector<vector<int> >(order.size(), vector<int>());
	g2.weights = vector<vector<double> >(order.size(), vector<double>());

	for (int ci = 0; ci < (int)order.size(); ci++)
	{
		int cIndex = order[ci];
		map<int, double> m;

		for (int i=0;i<(int)comm[cIndex].size();i++)
		{
			int vIndex = comm[cIndex][i];
			for (int i = 0; i < (int)g.links[vIndex].size(); i++)
			{
				int neigh = g.links[vIndex][i];
				int neigh_comm = comm2Order[n2c[neigh]];
				double neigh_weight = g.weights[vIndex][i];

				m[neigh_comm] += neigh_weight;
			}
		}

		g2.links[ci] = vector<int>(m.size());
		g2.weights[ci] = vector<double>(m.size());

		int wh = 0;
		for (auto iter=m.begin(); iter!=m.end(); iter++)
		{
			int it = (*iter).first;
			g2.totalWeight += m[it];
			g2.links[ci][wh] = it;
			g2.weights[ci][wh] = m[it];
			wh++;
		}
	}

	return g2;
}

Cluster* Community::prepareCluster(const Cluster* rootCluster, const BinaryGraph& binaryGraph) const
{
	Cluster* cluster = new Cluster(binaryGraph);
	map<int, vector<int> > comm;
	vector<int> order;
	for (int i = 0; i < size; i++)
	{
		if (comm.find(n2c[i])==comm.end())
		{
			comm[n2c[i]] = vector<int>();
			order.push_back(n2c[i]);
		}

		comm[n2c[i]].push_back(i);
	}

	for (int ci = 0; ci < (int)order.size(); ci++)
	{
		int cIndex = order[ci];
		BinaryGraph subBinaryGraph = rootCluster->constructSubBinaryGraph(comm[cIndex]);
		Cluster* subCluster = new Cluster(subBinaryGraph);
		for (int i=0;i<(int)comm[cIndex].size();i++)
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
