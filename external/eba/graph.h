#pragma once 

#include "common.h"
#include "point.h"
#include "rectangle.h"

class ConnectedGraph;

class Node
{
public:
	int index;

	string id;

	map<string, string> attr;

	Point pos;

	Node(int index): pos(-1.0, -1.0), index(index) {};

	Point getPos()
	{
		if (pos.x != -1 || pos.y != -1) return pos;

		if (attr.find("pos") == attr.end())
			cerr<<"No attribute 'pos' for the node '"<<id<<"'\n";

		string s = getAttr("pos");
		sscanf(s.c_str(), "%lf,%lf", &pos.x, &pos.y);
		return pos;
	}

	string getAttr(const string& key)
	{
		if (attr.find(key) == attr.end()) throw 1;
		//assert(attr.find(key) != attr.end());
		return attr[key];
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
		string s = getAttr(key);
		double v;
		sscanf(s.c_str(), "%lf", &v);
		return v;
	}
};

class Edge
{
public:
	int index;

	string s, t;
	map<string, string> attr;

	double len;

	Edge(int index): len(-1), index(index) {}

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
		//assert(attr.find(key) != attr.end());
		return attr[key];
	}

	double getDoubleAttr(const string& key)
	{
		string s = getAttr(key);
		double v;
		sscanf(s.c_str(), "%lf", &v);
		return v;
	}
};

class Graph
{
	bool initialized;

public:
	Graph(): initialized(false) {}

	vector<Node*> style;

	vector<Node*> nodes;
	vector<Edge*> edges;
	VVI adj;
	VVI adjE;
	map<pair<Node*, Node*>, Edge*> adjEdge;

	map<string, Node*> idToNode;
	//VI nodeToDegree;
	//VI nodeToDegree;
	map<Node*, int> nodeToDegree;
	map<Node*, double> nodeToWDegree;
	map<pair<Node*, Node*>, int> shortestPaths;
	map<pair<Node*, Node*>, double> shortestPathsW;

	void OutputStatistics()
	{
		printf("#nodes = %d  #edges = %d\n", nodes.size(), edges.size());
	}

	void assignClusters(vector<vector<Node*> >& clust, int first)
	{
		for (int i = 0; i < (int)clust.size(); i++)
		{
			string c = int2String(i + first + 1);
			for (int j = 0; j < (int)clust[i].size(); j++)
			{
				clust[i][j]->attr["cluster"] = c;
				clust[i][j]->removeAttr("clustercolor");
			}
		}
	}

	int numberOfClusters() const
	{
		set<string> cl;
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			if (!nodes[i]->hasAttr("cluster")) return -1;
			cl.insert(nodes[i]->getAttr("cluster"));
		}

		return (int)cl.size();
	}

	Node* findNodeById(const string& id)
	{
		if (idToNode.empty())
		{
			for (int i = 0; i < (int)nodes.size(); i++)
				idToNode[nodes[i]->id] = nodes[i];
		}

		return idToNode[id];
	}

	int degree(Node* node)
	{
		if (nodeToDegree.empty())
		{
			for (int i = 0; i < (int)edges.size(); i++)
			{
				Node* s = findNodeById(edges[i]->s);
				Node* t = findNodeById(edges[i]->t);

				nodeToDegree[s]++;
				nodeToDegree[t]++;
			}
		}

		return nodeToDegree[node];
	}

	double weightedDegree(Node* node)
	{
		if (nodeToWDegree.empty())
		{
			for (int i = 0; i < (int)edges.size(); i++)
			{
				Node* s = findNodeById(edges[i]->s);
				Node* t = findNodeById(edges[i]->t);

				nodeToWDegree[s] += edges[i]->getWeight();
				nodeToWDegree[t] += edges[i]->getWeight();
			}
		}

		return nodeToWDegree[node];
	}

	double getShortestPath(Node* s, Node* t, bool weighted)
	{
		pair<Node*, Node*> pair = make_pair(s, t);
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

	void initShortestPaths(Node* s, bool weighted);

	void initAdjacencyList()
	{
		if (initialized) return;

		adj = VVI(nodes.size(), VI());
		adjE = VVI(nodes.size(), VI());
		for (int i = 0; i < (int)edges.size(); i++)
		{
			Node* s = findNodeById(edges[i]->s);
			Node* t = findNodeById(edges[i]->t);

			adj[s->index].push_back(t->index);
			adjE[s->index].push_back(i);
			adj[t->index].push_back(s->index);
			adjE[t->index].push_back(i);

			adjEdge[make_pair(s, t)] = edges[i];
			adjEdge[make_pair(t, s)] = edges[i];
		}

		initialized = true;
	}

	Edge* findEdge(Node* v, Node* u)
	{
		pair<Node*, Node*> pr = make_pair(v, u);
		if (adjEdge.find(pr) == adjEdge.end()) return NULL;
		return adjEdge[pr];
	}

	vector<ConnectedGraph> getConnectedComponents();
};

class ConnectedGraph
{
	Graph g;

public:
	ConnectedGraph() {}
	ConnectedGraph(Graph& g, vector<Node*> nodes): g(g), nodes(nodes) {}

	vector<Node*> nodes;

	int maxNodeIndex() const
	{
		return g.nodes.size();
	}

	bool operator < (const ConnectedGraph& o) const
	{
		return (nodes.size() < o.nodes.size());
	}

	Edge* findEdge(Node* v, Node* u)
	{
		return g.findEdge(v, u);
	}

	vector<pair<Node*, Edge*> > getAdj(Node* v)
	{
		vector<pair<Node*, Edge*> > res;
		for (int i = 0; i < (int)g.adj[v->index].size(); i++)
		{
			Node* u = g.nodes[g.adj[v->index][i]];
			Edge* edge = g.edges[g.adjE[v->index][i]];

			res.push_back(make_pair(u, edge));
		}

		return res;
	}

	double weightedDegree(Node* node)
	{
		return g.weightedDegree(node);
	}

	double getShortestPath(Node* s, Node* t, bool weighted)
	{
		return g.getShortestPath(s, t, weighted);
	}

	Graph getOriginalGraph() const
	{
		return g;
	}
};

