#include "discover_community.h"

#include "cluster.h"
#include "community.h"

#include "common/geometry/point.h"
#include "common/geometry/delaunay_triangulation.h"

#include <iostream>

namespace modularity {

map<DotNode*, set<DotNode*> > SpatialNeighbors(DotGraph& g);

//remove stub(==with one child) subclusters
void postProcessClusters(Cluster* cluster)
{
	if (cluster->containsVertices())
		return;

	if (cluster->getSubClusters().size() == 1)
	{
		Cluster* subCluster = cluster->getSubClusters()[0];
		cluster->init(subCluster->getSubClusters(), subCluster->getVertexes());
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

Cluster* BuildHierarchy(BinaryGraph* binaryGraph, Cluster* rootCluster, bool contiguity)
{
	while (true)
	{
		Community c(binaryGraph, contiguity);

		double curModularity = c.modularity();
		double newModularity = c.one_level();
		binaryGraph = c.prepareBinaryGraph();
		rootCluster = c.prepareCluster(rootCluster);

		//cerr << "modularity increased from " << curModularity << " to " << newModularity << endl;
		if (Abs(newModularity - curModularity) < 1e-6)
			break;
	}

	postProcessClusters(rootCluster);
	return rootCluster;
}

BinaryGraph* CreateBinaryGraph(DotGraph& g, bool contiguity)
{
	// preprocess
	map<DotNode*, set<DotNode*> > neighbors = SpatialNeighbors(g);

	// initialize modularity graph
	BinaryGraph* g2 = new BinaryGraph((int)g.nodes.size(), contiguity);
	// initialize links: each link is counted twice
	for (int i = 0; i < g2->n; i++)
	{
		DotNode* v = g.nodes[i];
		vector<int> adj = g.adjE[v->index];
		for (int j = 0; j < (int)adj.size(); j++)
		{
			int nodeIndex = g.adj[v->index][j];
			int edgeIndex = g.adjE[v->index][j];
			DotEdge* edge = g.edges[edgeIndex];
			double weight = edge->getWeight();

			g2->links[i].push_back(nodeIndex);
			g2->weights[i].push_back(weight);
			g2->totalWeight += weight;
		}

		if (contiguity)
		{
			for (DotNode* u : neighbors[v])
				g2->adj[i].insert(u->index);
		}
	}

	g2->InitWeights();

	return g2;
}

vector<vector<DotNode*> > runModularity(DotGraph& g, bool contiguity)
{
	// initialize clusters
	Cluster* rootCluster = new Cluster();
	for (int i = 0; i < (int)g.nodes.size(); i++)
		rootCluster->addVertex(i);

	// run modularity
	rootCluster = BuildHierarchy(CreateBinaryGraph(g, contiguity), rootCluster, contiguity);

	vector<vector<DotNode*> > res;
	if (rootCluster->containsVertices())
	{
		vector<int> v = rootCluster->extractAllVertices();
		vector<DotNode*> resn;
		for (int i = 0; i < (int)v.size(); i++)
			resn.push_back(g.nodes[v[i]]);

		res.push_back(resn);
	}
	else
	{
		vector<Cluster*> cl = rootCluster->getSubClusters();
		for (int k = 0; k < (int)cl.size(); k++)
		{
			vector<int> v = cl[k]->extractAllVertices();
			vector<DotNode*> resn;
			for (int i = 0; i < (int)v.size(); i++)
				resn.push_back(g.nodes[v[i]]);

			res.push_back(resn);
		}
	}

	delete rootCluster;
	return res;
}

map<DotNode*, set<DotNode*> > SpatialNeighbors(DotGraph& g)
{
	if (g.nodes.empty())
		return map<DotNode*, set<DotNode*> >();

	//////////////////////////
	map<Point, DotNode*> posToNode;
	vector<Point> points;
	Rectangle bbRect(g.nodes[0]->getPos());
	for (auto node : g.nodes)
	{
		Rectangle bb = node->getBoundingRectangle();
		vector<Point> pp;
		pp.push_back(node->getPos());
		pp.push_back(Point(bb.xl, bb.yl));
		pp.push_back(Point(bb.xr, bb.yl));
		pp.push_back(Point(bb.xl, bb.yr));
		pp.push_back(Point(bb.xr, bb.yr));

		for (auto point : pp)
		{
			points.push_back(point);
			posToNode[point] = node;
			bbRect.Add(point);
		}
	}

	//boundary
	double sz = min(bbRect.getWidth(), bbRect.getHeight()) * 0.1;
	bbRect.Add(bbRect.minPoint() - Point(sz, sz));
	bbRect.Add(bbRect.maxPoint() + Point(sz, sz));

	points.push_back(Point(bbRect.xl, bbRect.yl));
	points.push_back(Point(bbRect.xl, bbRect.yr));
	points.push_back(Point(bbRect.xr, bbRect.yl));
	points.push_back(Point(bbRect.xr, bbRect.yr));

	auto dt = geometry::DelaunayTriangulation::Create(points);

	map<DotNode*, set<DotNode*> > neighbors;
	for (auto seg : dt->getSegments())
	{
		if (!posToNode.count(seg.first)) continue;
		if (!posToNode.count(seg.second)) continue;

		DotNode* an = posToNode[seg.first];
		DotNode* bn = posToNode[seg.second];
		if (an == bn) continue;

		neighbors[an].insert(bn);
		neighbors[bn].insert(an);
	}

	return neighbors;
}

} // namespace modularity
