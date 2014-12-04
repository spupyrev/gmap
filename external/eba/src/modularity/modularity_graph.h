#pragma once

#include <vector>
#include <algorithm>
using namespace std;

namespace modularity {

class ModularityVertex
{
	int index;

	vector<int> edges;
	vector<double> weights;

	ModularityVertex(const ModularityVertex& v);
	ModularityVertex& operator = (const ModularityVertex& v);

public:
	ModularityVertex(int index): index(index) {}

	inline int getEdge(int index) const
	{
		return edges[index];
	}

	inline double getWeight(int index) const
	{
		return weights[index];
	}

	inline int getEdgesSize() const
	{
		return (int)edges.size();
	}

	inline int getIndex() const
	{
		return index;
	}

	void initEdges(vector<int> v, vector<double> w)
	{
		edges = v;
		weights = w;
	}
};

class ModularityGraph
{
	vector<ModularityVertex*> vList;

	ModularityGraph(const ModularityGraph&);
	ModularityGraph& operator=(const ModularityGraph&);

public:
	ModularityGraph() {}

	~ModularityGraph()
	{
		for (int i = 0; i < (int)vList.size(); i++)
			delete vList[i];
	}

	inline int getSize() const 
	{
		return (int)vList.size();
	}

	inline ModularityVertex* getVertex(int index)
	{
		return vList[index];
	}

	ModularityVertex* addVertex()
	{
		ModularityVertex* newVertex = new ModularityVertex(getSize());
		vList.push_back(newVertex);
		return newVertex;
	}
};

} // namespace modularity
