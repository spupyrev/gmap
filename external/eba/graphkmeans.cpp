#include "common.h"
#include "clustering.h"

Node* GraphKMeans::getNextMean(const vector<Node*>& means, ConnectedGraph& g)
{
	VD minDist;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
        double minD = 123456789.0;
		for (int j = 0; j < (int)means.size(); j++)
        {
			double d = g.getShortestPath(g.nodes[i], means[j], true);
			minD = min(minD, d);
        }

		minDist.push_back(Sqr(minD));
    }

	int p = ChooseRandomWithProbability(minDist);
	if (minDist[p] < 1e-6) return NULL;

	return g.nodes[p];
}

vector<Node*> GraphKMeans::chooseCenters(ConnectedGraph& g, int K)
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

Node* GraphKMeans::computeMedian(const vector<Node*>& group, ConnectedGraph& g)
{
	double dmin = -1;
	int bestIndex = -1;
	for (int i = 0; i < (int)group.size(); i++)
	{
		double mx = -1;
		for (int j = 0; j < (int)group.size(); j++)
		{
			double d = g.getShortestPath(group[i], group[j], true);
			if (mx == -1 || mx < d) mx = d;
		}

		if (dmin == -1 || dmin > mx)
		{
			dmin = mx;
			bestIndex = i;
		}
	}

	return group[bestIndex];
}

vector<vector<Node*> > GraphKMeans::groupPoints(const vector<Node*>& means, ConnectedGraph& g)
{
	VVN groups = VVN(means.size(), VN());
	vector<Node*> median;
	for (int i = 0; i < (int)means.size(); i++)
		median.push_back(means[i]);

	for (int it = 0; it < 10; it++)
	{
		groups = VVN(median.size(), VN());

		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			double minDis = 123456789.0;
			int bestIndex = -1;

			for (int j = 0; j < (int)means.size(); j++)
			{
				double d = g.getShortestPath(g.nodes[i], means[j], true);
				if (minDis > d)
				{
					minDis = d;
					bestIndex = j;
				}
			}

			assert(bestIndex != -1);
			assert(minDis < 1234567.0);
			groups[bestIndex].push_back(g.nodes[i]);
		}

		bool progress = false;
		for (int i = 0; i < (int)median.size(); i++)
		{
			Node* newMedian = computeMedian(groups[i], g);
			if (median[i] != newMedian) progress = true;
			median[i] = newMedian;
		}

		if (!progress) break;
	}

    return groups;
}

vector<vector<Node*> > GraphKMeans::updateGroups(VVN& groups, ConnectedGraph& g)
{
	VI clusterSize;
	VI cl = VI(g.maxNodeIndex(), -1);
	for (int i = 0; i < (int)groups.size(); i++)
	{
		clusterSize.push_back(groups[i].size());
		for (int j = 0; j < (int)groups[i].size(); j++)
			cl[groups[i][j]->index] = i;
	}

	//reassign clusters
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

			//find best cluster
			int bestNewCluster = cl[v->index];
			for (int j = 0; j < (int)groups.size(); j++)
			{
				if (j == cl[v->index]) continue;
				if (clusterSize[j] == 0) continue;

				double wj = 1.0/sqrt((double)clusterSize[j]);
				double wb = 1.0/sqrt((double)clusterSize[bestNewCluster]);
				//double wj = 1.0/(double)clusterSize[j];
				//double wb = 1.0/(double)clusterSize[bestNewCluster];

				if (wSum[j]*wj > wSum[bestNewCluster]*wb) 
					bestNewCluster = j;
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

vector<vector<Node*> > GraphKMeans::cluster(ConnectedGraph& g, int K)
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

