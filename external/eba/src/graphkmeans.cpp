#include "common/common.h"
#include "common/random_utils.h"

#include "clustering.h"

DotNode* GraphKMeans::getNextMean(const vector<DotNode*>& means, ConnectedDotGraph& g)
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

		minDist.push_back(Sqr2(minD));
	}

	int p = randWithProbability(minDist);
	if (minDist[p] < 1e-6) return NULL;

	return g.nodes[p];
}

vector<DotNode*> GraphKMeans::chooseCenters(ConnectedDotGraph& g, int K)
{
	vector<DotNode*> centers;

	int i = randInt(g.nodes.size());
	centers.push_back(g.nodes[i]);

	for (int i = 1; i < K; i++)
	{
		DotNode* p = getNextMean(centers, g);
		if (p != NULL) centers.push_back(p);
	}

	return centers;
}

DotNode* GraphKMeans::computeMedian(const vector<DotNode*>& group, ConnectedDotGraph& g)
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

ClusteringInfo GraphKMeans::groupPoints(const vector<DotNode*>& means, ConnectedDotGraph& g)
{
	VVN groups = VVN(means.size(), VN());
	vector<DotNode*> median;
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
			DotNode* newMedian = computeMedian(groups[i], g);
			if (median[i] != newMedian) progress = true;
			median[i] = newMedian;
		}

		if (!progress) break;
	}

	return ClusteringInfo(&g, groups);
}

void GraphKMeans::updateClusters(ClusteringInfo& ci, ConnectedDotGraph& g)
{
	//reassign clusters
	bool progress = true;
	for (int t = 0; t < 30 && progress; t++)
	{
		progress = false;
		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			DotNode* v = g.nodes[i];
			double oldModularity = ci.getModularity();

			//try to find the best new cluster
			int bestCluster = ci.getCluster(v);
			double bestModularity = oldModularity;
			for (int j = 0; j < ci.getClusterCount(); j++)
			{
				if (j == ci.getCluster(v)) continue;

				ci.moveVertex(v, j);
				double newModularity = ci.getModularity();

				if (newModularity > bestModularity)
				{
					bestModularity = newModularity;
					bestCluster = j;
				}
			}

			if (bestCluster != ci.getCluster(v))
			{
				ci.moveVertex(v, bestCluster);
			}

			if (bestModularity > oldModularity) 
				progress = true;
		}
		//ci.checkConsistency();
	}
}

vector<vector<DotNode*> > GraphKMeans::cluster(ConnectedDotGraph& g, int K)
{
	assert(K >= 1);

	VVN res;
	double bestValue = -1;

	for (int attempt = 0; attempt < 10; attempt++)
	{
		vector<DotNode*> centers = chooseCenters(g, K);

		ClusteringInfo clusterInfo = groupPoints(centers, g);
		updateClusters(clusterInfo, g);

		double value = clusterInfo.getModularity();
		if (bestValue == -1 || bestValue < value)
		{
			bestValue = value;
			res = clusterInfo.getGroups();
		}
	}

	return res;
}

