#pragma once 

#include "common.h"
#include "graph.h"

typedef vector<Node*> VN;
typedef vector<VN> VVN;

vector<vector<Node*> > clusterBetweenness(const Graph& g, int K);

class ClusterAlgorithm
{
public:
	ClusterAlgorithm() {}
	~ClusterAlgorithm() {}

	virtual void cluster(Graph& g);
	virtual void cluster(Graph& g, int K);

protected:
	virtual VVN cluster(ConnectedGraph& g, int K) = 0;

	double modularity(ConnectedGraph& g, const VVN&) const;
	double modularity(ConnectedGraph& g, const VI&) const;
};

class GeometricKMeans: public ClusterAlgorithm
{
	VVN cluster(ConnectedGraph& g, int K);

	Node* getNextMean(const VN& means, ConnectedGraph& g);
	VN chooseCenters(ConnectedGraph& g, int K);
	VVN groupPoints(const VN& means, ConnectedGraph& g);
	Point computeMedian(const vector<Node*>& group);
	VVN updateGroups(VVN& groups, ConnectedGraph& g);
};

class GraphKMeans: public ClusterAlgorithm
{
	VVN cluster(ConnectedGraph& g, int K);

	Node* getNextMean(const VN& means, ConnectedGraph& g);
	VN chooseCenters(ConnectedGraph& g, int K);
	Node* computeMedian(const vector<Node*>& group, ConnectedGraph& g);
	VVN groupPoints(const VN& means, ConnectedGraph& g);
	VVN updateGroups(VVN& groups, ConnectedGraph& g);
};

class GeometricHierarchical: public ClusterAlgorithm
{
	VVN cluster(ConnectedGraph& g, int K);

	double computeAverageLength(ConnectedGraph& g, const VN& v1, const VN& v2);
};

class GraphHierarchical: public ClusterAlgorithm
{
	VVN cluster(ConnectedGraph& g, int K);

	double computeAverageLength(ConnectedGraph& g, const VN& v1, const VN& v2);
};

class InfoMap: public ClusterAlgorithm
{
	void cluster(Graph& g);
	void cluster(Graph& g, int K);
	VVN cluster(ConnectedGraph& g, int K) {return VVN();}
};
