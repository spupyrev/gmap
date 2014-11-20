#include "common/common.h"
#include "common/random_utils.h"
#include "common/geometry/point.h"

#include "clustering.h"

DotNode* GeometricKMeans::getNextMean(const vector<DotNode*>& means, ConnectedDotGraph& g)
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

	int p = randWithProbability(minDist);
	if (minDist[p] < 1e-6) return NULL;

	return g.nodes[p];
}

vector<DotNode*> GeometricKMeans::chooseCenters(ConnectedDotGraph& g, int K)
{
	vector<DotNode*> centers;

	int i = rand() % g.nodes.size();
	centers.push_back(g.nodes[i]);

	for (int i = 1; i < K; i++)
	{
		DotNode* p = getNextMean(centers, g);
		if (p != NULL) centers.push_back(p);
	}

	return centers;
}

Point GeometricKMeans::computeMedian(const vector<DotNode*>& group)
{
	Point sum;
	for (int i = 0; i < (int)group.size(); i++)
		sum += group[i]->getPos();

	if ((int)group.size() == 0) return sum;
	sum /= (double)group.size();

	return sum;
}

ClusteringInfo GeometricKMeans::groupPoints(const vector<DotNode*>& means, ConnectedDotGraph& g)
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
			DotNode* up = g.nodes[i];
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

	return ClusteringInfo(&g, groups);
}

void GeometricKMeans::updateClusters(ClusteringInfo& ci, ConnectedDotGraph& g)
{
	//find close vertices
	typedef pair<double, int> PR;
	vector<vector<PR> > nearV;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		DotNode* v = g.nodes[i];
		vector<PR> near;
		for (int j = 0; j < (int)g.nodes.size(); j++)
			if (i != j)
			{
				DotNode* u = g.nodes[j];
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
			DotNode* v = g.nodes[i];

			//check neighborhood
			vector<PR> near = nearV[i];
			VI cnt = VI(ci.getClusterCount(), 0);
			VI closeClusters;
			for (int j = 0; j < (int)near.size() && j < NEIGH; j++)
			{
				DotNode* u = g.nodes[near[j].second];
				int cluster = ci.getCluster(u);
				if (cnt[cluster] == 0) closeClusters.push_back(cluster);
				cnt[cluster]++;
			}

			double oldModularity = ci.getModularity();

			//find best cluster
			int bestCluster = ci.getCluster(v);
			double bestModularity = oldModularity;
			for (int j = 0; j < (int)closeClusters.size(); j++)
			{
				int cluster = closeClusters[j];
				if (cluster == ci.getCluster(v)) continue;

				//will break connectivity
				if (cnt[cluster] < MIN_EDGES) continue;

				ci.moveVertex(v, cluster);
				double newModularity = ci.getModularity();

				if (newModularity > bestModularity)
				{
					bestModularity = newModularity;
					bestCluster = cluster;
				}
			}

			if (bestCluster != ci.getCluster(v))
			{
				ci.moveVertex(v, bestCluster);
				//ci.checkConsistency();
			}

			if (bestModularity > oldModularity) 
				progress = true;
		}
	}
}

vector<vector<DotNode*> > GeometricKMeans::cluster(ConnectedDotGraph& g, int K)
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



