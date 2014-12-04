#include "discover_community.h"

#include "cluster.h"
#include "community.h"

#include <iostream>

namespace modularity {

#define MOD_DEBUG 0

double PRECISION = 1e-6;
int display_level = -2;

Cluster* doStep(Community c, BinaryGraph& g, double& modularity, Cluster* rootCluster, int clusterLimit = -1)
{
	if (MOD_DEBUG)
		cerr << "network: " << g.n << " nodes, " << g.totalWeight << " weight" << endl;

	double newModularity = c.one_level(rootCluster);

	if (MOD_DEBUG)
		cerr << "modularity increased from " << modularity << " to " << newModularity << endl;

	g = c.partition2graph_binary();

	rootCluster = c.prepareCluster(rootCluster, g);

	modularity = newModularity;
	return rootCluster;
}

//remove stub(==with one child) subclusters
void postProcessClusters(Cluster* cluster)
{
	if (cluster->containsVertices())
		return;

	if (cluster->getSubClusters().size() == 1)
	{
		Cluster* subCluster = cluster->getSubClusters()[0];
		cluster->init(subCluster->getSubClusters(), subCluster->getVertexes(), subCluster->getBinaryGraph());
		postProcessClusters(cluster);
	}
	else
	{
		for (int i = 0;i<(int)cluster->getSubClusters().size();i++)
		{
			Cluster* subCluster = cluster->getSubClusters()[i];
			postProcessClusters(subCluster);
		}
	}
}

Cluster* BuildHierarchy(BinaryGraph& binaryGraph, Cluster* rootCluster)
{
	Community c = Community(binaryGraph, -1, PRECISION);

	double modularity;
	double new_mod = c.modularity();

	do
	{
		modularity = new_mod;
		rootCluster = doStep(c, binaryGraph, new_mod, rootCluster);
		c = Community(binaryGraph, -1, PRECISION);
	}
	while (new_mod - modularity > PRECISION);

	if (MOD_DEBUG)
		cerr << "Discovering done: modularity = " << new_mod << endl;

	postProcessClusters(rootCluster);
	return rootCluster;
}

vector<vector<DotNode*> > runModularity(ConnectedDotGraph& g)
{
	// create modularity graph
	ModularityGraph graph;
	map<DotNode*, ModularityVertex*> n2v;
	map<ModularityVertex*, DotNode*> v2n;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		ModularityVertex* vertex = graph.addVertex();
		n2v[g.nodes[i]] = vertex;
		v2n[vertex] = g.nodes[i];
	}

	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		ModularityVertex* vertex = graph.getVertex(i);

		vector<int> v;
		vector<double> w;
		vector<pair<DotNode*, DotEdge*> > adj = g.getAdj(g.nodes[i]);
		for (int j = 0; j < (int)adj.size(); j++)
		{
			ModularityVertex* neig = n2v[adj[j].first];
			v.push_back(neig->getIndex());
			w.push_back(adj[j].second->getWeight());
		}

		vertex->initEdges(v, w);
	}
	   
	// initialize clusters
	BinaryGraph binaryGraph(graph);
	Cluster* rootCluster = new Cluster(binaryGraph);
	for (int i = 0; i < (int)graph.getSize(); i++)
	{
		ModularityVertex* v = graph.getVertex(i);
		rootCluster->addVertex(v);
	}

	// run modularity
	rootCluster = BuildHierarchy(binaryGraph, rootCluster);

	vector<vector<DotNode*> > res;
	if (rootCluster->containsVertices())
	{
		vector<ModularityVertex*> v = rootCluster->extractAllVertices();
		vector<DotNode*> resn;
		for (int i = 0; i < (int)v.size(); i++)
			resn.push_back(v2n[v[i]]);

		res.push_back(resn);
	}
	else
	{
		vector<Cluster*> cl = rootCluster->getSubClusters();
		for (int k = 0; k < (int)cl.size(); k++)
		{
			vector<ModularityVertex*> v = cl[k]->extractAllVertices();
			vector<DotNode*> resn;
			for (int i = 0; i < (int)v.size(); i++)
				resn.push_back(v2n[v[i]]);

			res.push_back(resn);
		}
	}

	delete rootCluster;

	return res;
}

} // namespace modularity
