#include "common.h"
#include "clustering.h"
#include "point.h"

int ChooseRandomWithProbabilityOld(VD& prob)
{
	int mx = -1;
	for (int i = 0; i < (int)prob.size(); i++)
		if (mx == -1 || prob[mx] < prob[i]) mx = i;
	return mx;
}

Node* GeometricKMeans::getNextMean(const vector<Node*>& means, ConnectedGraph& g)
{
	VD minDist;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
        double minD = 123456789.0;
		for (int j = 0; j < (int)means.size(); j++)
        {
            double d = g.nodes[i]->getPos().Distance(means[j]->getPos());
			minD = min(minD, d);
        }

		minDist.push_back(Sqr2(minD));
    }

	int p = ChooseRandomWithProbability(minDist);
	if (minDist[p] < 1e-6) return NULL;

	return g.nodes[p];
}

vector<Node*> GeometricKMeans::chooseCenters(ConnectedGraph& g, int K)
{
	vector<Node*> centers;

	int i = rand()%g.nodes.size();
	centers.push_back(g.nodes[i]);

	for (int i = 1; i < K; i++)
	{
		Node* p = getNextMean(centers, g);
		if (p != NULL) centers.push_back(p);
	}

	return centers;
}

Point GeometricKMeans::computeMedian(const vector<Node*>& group)
{
	Point sum;
	for (int i = 0; i < (int)group.size(); i++)
		sum += group[i]->getPos();

	if ((int)group.size() == 0) return sum;
	sum /= (double)group.size();

    return sum;
}

vector<vector<Node*> > GeometricKMeans::groupPoints(const vector<Node*>& means, ConnectedGraph& g)
{
	VVN groups;
	vector<Point> median;
	for (int i = 0; i < (int)means.size(); i++)
		median.push_back(means[i]->getPos());

	for (int it = 0; it < 10; it++)
	{
		groups = VVN(median.size(), VN());

		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			Node* up = g.nodes[i];
			double minDis = 123456789.0;

			int bestIndex = -1;
			for (int j = 0; j < (int)median.size(); j++)
			{
				double d = up->getPos().Distance(median[j]);
				if (minDis > d)
				{
					minDis = d;
					bestIndex = j;
				}
			}

			assert (bestIndex != -1);
			groups[bestIndex].push_back(up);
		}

		for (int i = 0; i < (int)median.size(); i++)
			median[i] = computeMedian(groups[i]);
	}

	return groups;
}

vector<vector<Node*> > GeometricKMeans::updateGroups(VVN& groups, ConnectedGraph& g)
{
	VI clusterSize;
	VI cl = VI(g.maxNodeIndex(), -1);
	for (int i = 0; i < (int)groups.size(); i++)
	{
		clusterSize.push_back(groups[i].size());
		for (int j = 0; j < (int)groups[i].size(); j++)
			cl[groups[i][j]->index] = i;
	}

	//find close vertices
	typedef pair<double, int> PR;
	vector<vector<PR> > nearV;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		Node* v = g.nodes[i];
		vector<PR> near;
		for (int j = 0; j < (int)g.nodes.size(); j++)
			if (i != j)
			{
				Node* u = g.nodes[j];
				double dist = v->getPos().Distance(u->getPos());
				near.push_back(make_pair(dist, j));
			}

		sort(near.begin(), near.end());
		nearV.push_back(near);
	}

	//reassign clusters
	int NEIGH = 10;
	int MIN_EDGES = 4;
	bool progress = true;
	for (int t = 0; t < 30 && progress; t++)
	{
		progress = false;
		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			Node* v = g.nodes[i];
			//weight in i-th cluster
			VD wSum = VD(groups.size(), 0);
			vector<pair<Node*, Edge*> > adj = g.getAdj(v);
			for (int j = 0; j < (int)adj.size(); j++)
			{
				Node* u = adj[j].first;
				Edge* edge = adj[j].second;
				wSum[cl[u->index]] += edge->getWeight();
			}

			vector<PR> near = nearV[i];
			VI cnt = VI(groups.size(), 0);
			VI closeClusters;
			for (int j = 0; j < (int)near.size() && j < NEIGH; j++)
			{
				Node* u = g.nodes[near[j].second];
				int cluster = cl[u->index];
				if (cnt[cluster] == 0) closeClusters.push_back(cluster);
				cnt[cluster]++;
			}

			//find best cluster
			int bestNewCluster = cl[v->index];
			for (int j = 0; j < (int)closeClusters.size(); j++)
			{
				int cluster = closeClusters[j];

				if (cluster == cl[v->index]) continue;
				if (clusterSize[cluster] == 0) continue;

				//will break connectivity
				if (cnt[cluster] < MIN_EDGES) continue;

				double wj = 1.0;///(double)clusterSize[cluster];
				double wb = 1.0;//(double)clusterSize[bestNewCluster];

				if (wSum[cluster]*wj > wSum[bestNewCluster]*wb) 
					bestNewCluster = cluster;
			}

			if (bestNewCluster != cl[v->index])
			{
				clusterSize[cl[v->index]]--;
				clusterSize[bestNewCluster]++;

				cl[v->index] = bestNewCluster;
				progress = true;
			}
		}
	}

	vector<vector<Node*> > groupsRes = vector<vector<Node*> >(groups.size(), vector<Node*>());
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		Node* v = g.nodes[i];
		groupsRes[cl[v->index]].push_back(v);
	}

    return groupsRes;
}

vector<vector<Node*> > GeometricKMeans::cluster(ConnectedGraph& g, int K)
{
	assert(K >= 1);

	VVN res;
	double bestValue = -1;

	for (int attempt = 0; attempt < 10; attempt++)
	{
		vector<Node*> centers = chooseCenters(g, K);

		vector<vector<Node*> > groups = groupPoints(centers, g);
		groups = updateGroups(groups, g);

		vector<vector<Node*> > resGroups;
		for (int i = 0; i < (int)groups.size(); i++)
			if (groups[i].size() > 0) resGroups.push_back(groups[i]);

		double value = clusterQuality(g, resGroups);
		if (bestValue == -1 || bestValue < value)
		{
			bestValue = value;
			res = resGroups;
		}
	}

	return res;
}



