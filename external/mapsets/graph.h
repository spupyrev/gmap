#pragma once 
#ifndef GRAPH_H_
#define GRAPH_H_

#include "common.h"
#include "point.h"
#include "segment.h"
#include "rectangle.h"

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

	void setPos(const Point& p)
	{
		char s[128];
		sprintf(s, "%.3lf,%.3lf", p.x, p.y);
		attr["pos"] = string(s);
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
		string s = getAttr(key);
		double v;
		sscanf(s.c_str(), "%lf", &v);
		return v;
	}

//#define SCALE 72.0
	#define SCALE 52.0

	double getWidth()
	{
		assert(hasAttr("width"));
		return getDoubleAttr("width") * SCALE;
	}

	string getCluster()
	{
		assert(hasAttr("cluster"));
		return getAttr("cluster");
	}

	double getHeight()
	{
		assert(hasAttr("height"));
		return getDoubleAttr("height") * SCALE;
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

class Edge
{
public:
	int index;

	string s, t;
	map<string, string> attr;

	Edge(int index): index(index) {}

	string getAttr(const string& key)
	{
		if (attr.find(key) == attr.end()) throw 1;
		return attr[key];
	}

	void setAttr(const string& key, const string& value)
	{
		attr[key] = value;
	}
};

class Graph
{
public:
	Graph() {}

	vector<Node*> style;

	vector<Node*> nodes;
	vector<Edge*> edges;

	map<string, vector<Node*> > clusters;

	Node* AddNode()
	{
		Node* n = new Node(nodes.size());
		char s[128];
		sprintf(s, "%d-dummy", n->index);
		n->id = string(s);
		nodes.push_back(n);

		return n;
	}

	void AddDummyPoint(const Point& pos, const string& clusterId)
	{
		Node* nn = AddNode();
		nn->setPos(pos);
		nn->setAttr("cluster", clusterId);
		nn->setAttr("height", "0.0");
		nn->setAttr("width", "0.0");
		nn->setAttr("shape", "point");
		nn->setAttr("style", "invis");
	}

	vector<Node*> GetCluster(const string& clusterId)
	{
		if (clusters.empty()) InitClusters();

		return clusters[clusterId];
	}

	vector<Point> GetClusterPositions(const string& clusterId)
	{
		vector<Node*> cl = GetCluster(clusterId);
		vector<Point> pp;
		for (int i = 0; i < (int)cl.size(); i++)
			pp.push_back(cl[i]->getPos());

		return pp;
	}

	map<string, vector<Node*> > GetClusters()
	{
		if (clusters.empty()) InitClusters();

		return clusters;
	}

	int GetClusterCount()
	{
		if (clusters.empty()) InitClusters();

		return (int)clusters.size();
	}

	void InitClusters()
	{
		if (!clusters.empty()) return;

		for (int i = 0; i < (int)nodes.size(); i++)
		{
			Node* node = nodes[i];
			assert(node->hasAttr("cluster"));
			clusters[node->getAttr("cluster")].push_back(node);
		}
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
};

#endif
