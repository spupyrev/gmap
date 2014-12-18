#pragma once

#include <cassert>
#include <vector>
using namespace std;

namespace modularity {

class Cluster
{
private:
	vector<Cluster*> subClusters;
	vector<int> vertexes;

	Cluster(const Cluster&);
	Cluster& operator = (const Cluster&);

public:
	Cluster() {}

	~Cluster()
	{
		for (int i = 0; i < (int)subClusters.size(); i++)
			delete subClusters[i];
	}

	inline void init(const vector<Cluster*>& subClusters, const vector<int>& vertexes)
	{
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

	inline const vector<int>& getVertexes() const
	{
		return vertexes;
	}

	inline const vector<Cluster*>& getSubClusters() const
	{
		return subClusters;
	}

	inline void addVertex(int v)
	{
		assert(subClusters.empty());

		vertexes.push_back(v);
	}

	inline void addSubCluster(Cluster* c)
	{
		assert(vertexes.empty());

		subClusters.push_back(c);
	}

	vector<int> extractAllVertices() const;
};

} // namespace modularity
