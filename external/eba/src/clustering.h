#pragma once

#include "common/graph/dot_graph.h"

typedef vector<DotNode*> VN;
typedef vector<VN> VVN;

class ClusterAlgorithm
{
public:
	ClusterAlgorithm() {}
	virtual ~ClusterAlgorithm() {}

	virtual void cluster(DotGraph& g);
	virtual void cluster(DotGraph& g, int K);

protected:
	virtual VVN cluster(ConnectedDotGraph& g, int K) = 0;
};

class ClusteringInfo
{
	ConnectedDotGraph* g;
	//index of node cluster, or -1 if the node is not present in G
	vector<int> cluster;
	int clusterCount;
	double modularity;

	//twice the sum of all edge weights
	double m2;

	//sum of weights of all edges incident to a node in the cluster; inner edges are counted twice
	vector<double> sumTot;

	//sum of weights of all edges inside the cluster
	vector<double> sumIn;

public:
	ClusteringInfo(ConnectedDotGraph* g, const VVN& groups);

	inline int getClusterCount() const
	{
		return clusterCount;
	}

	inline int getCluster(const DotNode* v) const
	{
		return cluster[v->index];
	}

	inline double getModularity() const
	{
		return modularity;
	}

	VVN getGroups() const;
	void moveVertex(const DotNode* v, int newCluster);
	void checkConsistency();

private:
	double getModularitySlow() const;
	double computeKiIn(const DotNode* v, int clusterId) const;
};

class GeometricKMeans: public ClusterAlgorithm
{
	VVN cluster(ConnectedDotGraph& g, int K);

	DotNode* getNextMean(const VN& means, ConnectedDotGraph& g);
	VN chooseCenters(ConnectedDotGraph& g, int K);
	Point computeMedian(const vector<DotNode*>& group);

	ClusteringInfo groupPoints(const VN& means, ConnectedDotGraph& g);
	void updateClusters(ClusteringInfo& ci, ConnectedDotGraph& g);
};

class GraphKMeans: public ClusterAlgorithm
{
	VVN cluster(ConnectedDotGraph& g, int K);

	DotNode* getNextMean(const VN& means, ConnectedDotGraph& g);
	VN chooseCenters(ConnectedDotGraph& g, int K);
	DotNode* computeMedian(const vector<DotNode*>& group, ConnectedDotGraph& g);

	ClusteringInfo groupPoints(const VN& means, ConnectedDotGraph& g);
	void updateClusters(ClusteringInfo& ci, ConnectedDotGraph& g);
};

class GeometricHierarchical: public ClusterAlgorithm
{
	VVN cluster(ConnectedDotGraph& g, int K);

	double computeAverageLength(ConnectedDotGraph& g, const VN& v1, const VN& v2);
};

class GraphHierarchical: public ClusterAlgorithm
{
	VVN cluster(ConnectedDotGraph& g, int K);

	double computeAverageLength(ConnectedDotGraph& g, const VN& v1, const VN& v2);
};

class InfoMap: public ClusterAlgorithm
{
	void cluster(DotGraph& g);
	void cluster(DotGraph& g, int K);
	VVN cluster(ConnectedDotGraph& g, int K)
	{
		return VVN();
	}
};

class Modularity: public ClusterAlgorithm
{
	bool contigous;
public:
	Modularity(bool contigous): contigous(contigous) {}

	void cluster(DotGraph& g);
	void cluster(DotGraph& g, int K);
	VVN cluster(ConnectedDotGraph& g, int K)
	{
		return VVN();
	}
};
