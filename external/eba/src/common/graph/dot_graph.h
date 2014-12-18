#pragma once

#include "common/geometry/point.h"
#include "common/geometry/segment.h"
#include "common/geometry/rectangle.h"

#include <set>
#include <map>
#include <iostream>
#include <cassert>

class ConnectedDotGraph;

class DotNode
{
private:
	DotNode(const DotNode&);
	DotNode& operator = (const DotNode&);

public:
	int index;
	string id;
	map<string, string> attr;
	Point pos;

	DotNode(int index): index(index), pos(-1.0, -1.0) {};

	Point getPos()
	{
		if (pos.x != -1 || pos.y != -1) return pos;

		if (attr.find("pos") == attr.end())
			cerr << "No attribute 'pos' for the node '" << id << "'\n";

		char ch;
		istringstream (getAttr("pos")) >> pos.x >> ch >> pos.y;
		return pos;
	}

	string getCluster()
	{
		assert(hasAttr("cluster"));
		return getAttr("cluster");
	}

	#define SCALE 52.0

	double getWidth()
	{
		assert(hasAttr("width"));
		return getDoubleAttr("width") * SCALE;
	}

	double getHeight()
	{
		assert(hasAttr("height"));
		return getDoubleAttr("height") * SCALE;
	}

	string getAttr(const string& key)
	{
		assert(attr.find(key) != attr.end());
		return attr[key];
	}

	void setAttr(const string& key, const string& value)
	{
		attr[key] = value;
	}

	bool hasAttr(const string& key)
	{
		return (attr.find(key) != attr.end());
	}

	void removeAttr(const string& key)
	{
		if (attr.find(key) != attr.end())
			attr.erase(attr.find(key));
	}

	double getDoubleAttr(const string& key)
	{
		return toDouble(getAttr(key));
	}

	vector<Segment> getBoundary(double marginCoef)
	{
		assert(marginCoef >= 1.0);

		Point p = getPos();
		double width = getWidth();
		double height = getHeight();

		double delta = min((marginCoef - 1.0)*height, (marginCoef - 1.0)*width);
		width += delta;
		height += delta;


		p -= Point(width/2, height/2);
		vector<Segment> boundary;
		boundary.push_back(Segment(Point(p.x, p.y), Point(p.x + width, p.y)));
		boundary.push_back(Segment(Point(p.x + width, p.y), Point(p.x + width, p.y + height)));
		boundary.push_back(Segment(Point(p.x + width, p.y + height), Point(p.x, p.y + height)));
		boundary.push_back(Segment(Point(p.x, p.y + height), Point(p.x, p.y)));

		return boundary;
	}

	Rectangle getBoundingRectangle()
	{
		Point p = getPos();
		if (!hasAttr("width") || !hasAttr("height"))
			return Rectangle(p);

		double width = getWidth();
		double height = getHeight();

		p -= Point(width/2, height/2);
		return Rectangle(p.x, p.x + width, p.y, p.y + height);
	}

	bool IsDummy()
	{
		return (hasAttr("shape") && getAttr("shape") == "point");
	}
};

class DotEdge
{
private:
	DotEdge(const DotEdge&);
	DotEdge& operator = (const DotEdge&);

public:
	int index;
	string s, t;
	map<string, string> attr;
	double len;

	DotEdge(int index): index(index), len(-1) {}

	double getWeight()
	{
		if (attr.find("weight") == attr.end()) return 1.0;
		return getDoubleAttr("weight");
	}

	double getLen()
	{
		if (len != -1) return len;

		if (attr.find("len") == attr.end()) len = 1;
		else len = getDoubleAttr("len");

		return len;
	}

	string getAttr(const string& key)
	{
		if (attr.find(key) == attr.end()) throw 1;
		return attr[key];
	}

	double getDoubleAttr(const string& key)
	{
		return toDouble(getAttr(key));
	}
};

class DotGraph
{
	friend class ConnectedDotGraph;

	bool initialized;

public:
	DotGraph(): initialized(false) {}

	vector<DotNode*> style;

	vector<DotNode*> nodes;
	vector<DotEdge*> edges;

	//make it all mutable!!!
	map<string, vector<DotNode*> > clusters;
	vector<vector<int> > adj;
	vector<vector<int> > adjE;
	map<pair<DotNode*, DotNode*>, DotEdge*> adjEdge;

	map<string, DotNode*> idToNode;
	vector<int> nodeDegree;
	vector<double> nodeWDegree;
	map<pair<DotNode*, DotNode*>, int> shortestPaths;
	map<pair<DotNode*, DotNode*>, double> shortestPathsW;

	void AddDummyPoint(const Point& pos, const string& clusterId)
	{
		DotNode* nn = new DotNode(nodes.size());

		stringstream ssIndex;
		ssIndex << nn->index << "-dummy";
		nn->id = ssIndex.str();

		stringstream ss;
		ss << pos.x << "," << pos.y;
		nn->attr["pos"] = ss.str();
		nn->setAttr("cluster", clusterId);
		nn->setAttr("height", "0.0");
		nn->setAttr("width", "0.0");
		nn->setAttr("shape", "point");
		nn->setAttr("style", "invis");

		nodes.push_back(nn);
	}

	void OutputStatistics()
	{
		cout << "#nodes = " << nodes.size() << "  #edges = " << edges.size() << "  #clusters = " << ClusterCount() << endl;
	}

	void AssignClusters(vector<vector<DotNode*> >& clust, int first)
	{
		for (int i = 0; i < (int)clust.size(); i++)
		{
			string c = to_string(i + first + 1);
			for (int j = 0; j < (int)clust[i].size(); j++)
			{
				clust[i][j]->attr["cluster"] = c;
				clust[i][j]->removeAttr("clustercolor");
			}
		}
	}

	int ClusterCount() const
	{
		set<string> cl;
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			if (!nodes[i]->hasAttr("cluster")) return -1;
			cl.insert(nodes[i]->getAttr("cluster"));
		}

		return (int)cl.size();
	}

	vector<DotNode*> GetCluster(const string& clusterId)
	{
		if (clusters.empty()) 
			InitClusters();

		return clusters[clusterId];
	}

	void InitClusters()
	{
		if (!clusters.empty()) return;

		for (int i = 0; i < (int)nodes.size(); i++)
		{
			DotNode* node = nodes[i];
			assert(node->hasAttr("cluster"));
			clusters[node->getAttr("cluster")].push_back(node);
		}
	}

	vector<Point> GetClusterPositions(const string& clusterId)
	{
		vector<DotNode*> cl = GetCluster(clusterId);
		vector<Point> pp;
		for (int i = 0; i < (int)cl.size(); i++)
			pp.push_back(cl[i]->getPos());

		return pp;
	}

	map<string, vector<DotNode*> > GetClusters()
	{
		if (clusters.empty()) InitClusters();

		return clusters;
	}

	DotNode* findNodeById(const string& id)
	{
		if (idToNode.empty())
		{
			for (int i = 0; i < (int)nodes.size(); i++)
				idToNode[nodes[i]->id] = nodes[i];
		}

		return idToNode[id];
	}

	int degree(const DotNode* node)
	{
		if (nodeDegree.empty())
		{
			nodeDegree = VI(nodes.size(), 0);
			for (int i = 0; i < (int)edges.size(); i++)
			{
				DotNode* s = findNodeById(edges[i]->s);
				DotNode* t = findNodeById(edges[i]->t);

				nodeDegree[s->index]++;
				nodeDegree[t->index]++;
			}
		}

		return nodeDegree[node->index];
	}

	double weightedDegree(const DotNode* node)
	{
		if (nodeWDegree.empty())
		{
			nodeWDegree = VD(nodes.size(), 0);
			for (int i = 0; i < (int)edges.size(); i++)
			{
				DotNode* s = findNodeById(edges[i]->s);
				DotNode* t = findNodeById(edges[i]->t);

				nodeWDegree[s->index] += edges[i]->getWeight();
				nodeWDegree[t->index] += edges[i]->getWeight();
			}
		}

		return nodeWDegree[node->index];
	}

	double getShortestPath(DotNode* s, DotNode* t, bool weighted)
	{
		pair<DotNode*, DotNode*> pair = make_pair(s, t);
		if (weighted)
		{
			if (shortestPathsW.find(pair) == shortestPathsW.end())
				initShortestPaths(s, true);

			return shortestPathsW[pair];
		}
		else
		{
			if (shortestPaths.find(pair) == shortestPaths.end())
				initShortestPaths(s, false);

			return shortestPaths[pair];
		}
	}

	void initShortestPaths(DotNode* s, bool weighted);

	void initAdjacencyList()
	{
		if (initialized) return;

		adj = VVI(nodes.size(), VI());
		adjE = VVI(nodes.size(), VI());
		for (int i = 0; i < (int)edges.size(); i++)
		{
			DotNode* s = findNodeById(edges[i]->s);
			DotNode* t = findNodeById(edges[i]->t);

			adj[s->index].push_back(t->index);
			adjE[s->index].push_back(i);
			adj[t->index].push_back(s->index);
			adjE[t->index].push_back(i);

			adjEdge[make_pair(s, t)] = edges[i];
			adjEdge[make_pair(t, s)] = edges[i];
		}

		initialized = true;
	}

	DotEdge* findEdge(DotNode* v, DotNode* u)
	{
		pair<DotNode*, DotNode*> pr = make_pair(v, u);
		if (adjEdge.find(pr) == adjEdge.end()) return NULL;
		return adjEdge[pr];
	}

	Rectangle GetBoundingBox() const
	{
		Rectangle bb(nodes[0]->getPos());
		for (int i = 1; i < (int)nodes.size(); i++)
		{
			bb.Add(nodes[i]->getPos());
		}

		double K = 1.2;
		double w = bb.xr - bb.xl;
		double h = bb.yr - bb.yl;
		bb.xl -= K*w;
		bb.xr += K*w;
		bb.yl -= K*h;
		bb.yr += K*h;
		return bb;
	}

	vector<ConnectedDotGraph> getConnectedComponents();
};

class ConnectedDotGraph
{
	friend class DotGraph;

	DotGraph g;

public:
	ConnectedDotGraph() {}
	ConnectedDotGraph(DotGraph& g, vector<DotNode*> nodes): g(g), nodes(nodes) 
	{
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			DotNode* v = nodes[i];
			for (int j = 0; j < (int)g.adj[v->index].size(); j++)
			{
				int adjIndex = g.adj[v->index][j];
				DotNode* u = g.nodes[adjIndex];
				if (v->index > u->index) continue;

				edges.push_back(g.findEdge(v, u));
			}
		}
	}

	vector<DotNode*> nodes;
	vector<DotEdge*> edges;

	int maxNodeIndex() const
	{
		return g.nodes.size();
	}

	bool operator < (const ConnectedDotGraph& o) const
	{
		return (nodes.size() < o.nodes.size());
	}

	DotNode* findNodeById(const string& id)
	{
		return g.findNodeById(id);
	}

	DotEdge* findEdge(DotNode* v, DotNode* u)
	{
		return g.findEdge(v, u);
	}

	vector<pair<DotNode*, DotEdge*> > getAdj(const DotNode* v)
	{
		vector<pair<DotNode*, DotEdge*> > res;
		for (int i = 0; i < (int)g.adj[v->index].size(); i++)
		{
			DotNode* u = g.nodes[g.adj[v->index][i]];
			DotEdge* DotEdge = g.edges[g.adjE[v->index][i]];

			res.push_back(make_pair(u, DotEdge));
		}

		return res;
	}

	double weightedDegree(const DotNode* node)
	{
		return g.weightedDegree(node);
	}

	double getShortestPath(DotNode* s, DotNode* t, bool weighted)
	{
		return g.getShortestPath(s, t, weighted);
	}

	DotGraph getOriginalGraph() const
	{
		return g;
	}
};

