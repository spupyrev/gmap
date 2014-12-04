#pragma once

#include "binary_graph.h"

#include <cassert>
#include <vector>
using namespace std;

namespace modularity {

class Cluster
{
private:
	vector<Cluster*> subClusters;
	vector<ModularityVertex*> vertexes;
	BinaryGraph binaryGraph;

	Cluster(const Cluster& v);
	Cluster& operator = (const Cluster& v);

public:
	Cluster(BinaryGraph binaryGraph): binaryGraph(binaryGraph) {}

	~Cluster()
	{
		for (int i=0;i<(int)subClusters.size();i++)
			delete subClusters[i];
	}

	inline void init(const vector<Cluster*>& subClusters, const vector<ModularityVertex*>& vertexes, const BinaryGraph& binaryGraph)
	{
		this->binaryGraph = binaryGraph;
		this->subClusters = subClusters;
		this->vertexes = vertexes;
	}

	inline bool containsVertices() const
	{
		return (!vertexes.empty());
	}

	inline bool containsClusters() const
	{
		return (!containsVertices());
	}

	inline const vector<ModularityVertex*>& getVertexes() const
	{
		return vertexes;
	}

	inline const vector<Cluster*>& getSubClusters() const
	{
		return subClusters;
	}

	inline BinaryGraph getBinaryGraph() const
	{
		return binaryGraph;
	}

	inline void addVertex(ModularityVertex* v)
	{
		assert(subClusters.empty());

		vertexes.push_back(v);
	}

	inline void addSubCluster(Cluster* c)
	{
		assert(vertexes.empty());

		subClusters.push_back(c);
	}

	int getSize() const;

	BinaryGraph constructSubBinaryGraph(const vector<int>& indexes) const;

	vector<ModularityVertex*> extractAllVertices() const;
};

} // namespace modularity
