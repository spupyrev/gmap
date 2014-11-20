#include "common/geometry/geometry_utils.h"

#include "visibility.h"
#include "graph_algorithms.h"
#include "rtree.h"
#include "closest_point.h"

typedef vector<Point> VP;

vector<VV> CreateVisibilityVertices(const vector<Point>& p, const vector<Segment>& obstacles);
vector<vector<int> > CreateVisibilityEdges(const vector<VV>& vis, const vector<Segment>& obstacles);

bool IsVisible(const VV& s, const VV& t, const vector<Segment>& obstacles, RTree* obstacleTreeRoot);
bool IsInCone(const VV& cone, const Point& p);
bool Intersect(const VV& s, const VV& t, const Segment& seg);

void VisibilityGraph::Initialze(const vector<Point>& p, const vector<Segment>& obstacles)
{
	nodes = CreateVisibilityVertices(p, obstacles);
	edges = CreateVisibilityEdges(nodes, obstacles);
}

struct ClockwiseComparator
{
	Point origin;
	Point head;

	bool operator() (const Point& p1, const Point& p2) 
	{ 
		return geometry::OrientationOf3Vectors(head - origin, p1 - origin, p2 - origin) <= 0;
	}
} comparator;

vector<VV> CreateVisibilityVertices(const vector<Point>& p, const vector<Segment>& obstacles)
{
	vector<VV> res;
	//adding regular points
	for (int i = 0; i < (int)p.size(); i++)
	{
		res.push_back(VV(p[i], true, res.size()));
	}

	//adding obstacles corners
	map<Point, set<Point> > neig;
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		Point p = obstacles[i].first;
		Point q = obstacles[i].second;

		neig[p].insert(q);
		neig[q].insert(p);
	}

	for (auto iter = neig.begin(); iter != neig.end(); iter++)
	{
		Point p = (*iter).first;
		VP adj = VP((*iter).second.begin(), (*iter).second.end());
		assert(!adj.empty());

		//sort clockwise
		comparator.origin = p;
		comparator.head = adj[0];
		sort(adj.begin(), adj.end(), comparator);


		for (int i = 0; i < (int)adj.size(); i++)
		{
			VV v(p, false, res.size());
			v.leftP = adj[i];
			v.rightP = adj[(i+1)%adj.size()];

			res.push_back(v);
		}
	}

	return res;
}

Rectangle SegmentBoundingBox(const Segment& seg)
{
	double minX = min(seg.first.x, seg.second.x);
	double maxX = max(seg.first.x, seg.second.x);
	double minY = min(seg.first.y, seg.second.y);
	double maxY = max(seg.first.y, seg.second.y);

	return Rectangle(minX, maxX, minY, maxY);
}

vector<vector<int> > CreateVisibilityEdges(const vector<VV>& vis, const vector<Segment>& obstacles)
{
	if (obstacles.empty())
	{
		vector<vector<int> > edges = VVI(vis.size(), VI());
		for (int i = 0; i < (int)vis.size(); i++)
		{
			for (int j = 0; j < (int)vis.size(); j++)
				if (i != j)
				{
					edges[i].push_back(j);
				}
		}

		return edges;
	}

	vector<RNode*> nodes;
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		RNode* node = new RNode(SegmentBoundingBox(obstacles[i]), obstacles[i]);
		nodes.push_back(node);
	}

	RTree* obstacleTree = new RTree(nodes);
	assert(obstacleTree->getRoot() != NULL);

	vector<vector<int> > edges = VVI(vis.size(), VI());

	//full visibility
	for (int i = 0; i < (int)vis.size(); i++)
	{
		for (int j = 0; j < (int)vis.size(); j++)
			if (i != j)
			{
				if (IsVisible(vis[i], vis[j], obstacles, obstacleTree)) 
					edges[i].push_back(j);
			}
	}

	delete obstacleTree;
	for (int i = 0; i < (int)obstacles.size(); i++)
		delete nodes[i];

	/*
	//sparse visibility
	vector<int> adj;
	Rectangle bb;
	for (int i = 0; i < (int)vis.size(); i++)
	{
		adj.push_back(i);
		bb.Add(vis[i].p);
	}

	ClosestPointQP cp(bb);
	map<Point, vector<int> > point2VV;
	for (int i = 0; i < (int)vis.size(); i++)
	{
		if (point2VV.find(vis[i].p) == point2VV.end())
			cp.addPoint(vis[i].p);

		point2VV[vis[i].p].push_back(i);
	}

	//OutputTimeInfo("#nodes: %d", vis.size());
	int maxAdj = 30;
	for (int i = 0; i < (int)vis.size(); i++)
	{
		vector<Point> adjP = cp.getKClosest(vis[i].p, maxAdj, 64);
		//OutputTimeInfo("#extracted nodes: %d", adjP.size());

		for (int k = 0; k < (int)adjP.size(); k++)
		{
			vector<int>& adj = point2VV[adjP[k]];
			for (int j = 0; j < (int)adj.size(); j++)
			{
				if (i == adj[j]) continue;
				if (IsVisible(vis[i], vis[adj[j]], obstacles, obstacleTreeRoot)) 
					edges[i].push_back(adj[j]);
			}
		}
	}
	*/
	
	//OutputTimeInfo("edges computed");
	return edges;
}

bool IntersectRectangleWithRNode(RNode* node, const Rectangle& rect, const VV& s, const VV& t)
{
	if (!node->getRectangle().intersects(rect)) return false;

	if (!node->IsLeaf()) 
	{
		return IntersectRectangleWithRNode(node->getLeft(), rect, s, t) || IntersectRectangleWithRNode(node->getRight(), rect, s, t);
    }
    else 
	{
		if (Intersect(s, t, node->getData())) return true;
    }

	return false;
}

bool IsVisible(const VV& s, const VV& t, const vector<Segment>& obstacles, RTree* obstacleTree)
{
	if (s.p == t.p) return false;

	//check crossings with obstacles
	if (IntersectRectangleWithRNode(obstacleTree->getRoot(), SegmentBoundingBox(Segment(s.p, t.p)), s, t)) return false;

	if (!s.real && !t.real)
	{
		if (s.leftP == t.p && t.rightP == s.p) return true;
		if (s.rightP == t.p && t.leftP == s.p) return true;
	}

	//check cones
	if (!IsInCone(s, t.p) || !IsInCone(t, s.p)) return false;

	return true;
}

bool IsInCone(const VV& cone, const Point& p)
{
	if (cone.real) return true;
	if (cone.leftP == cone.rightP) return true;

	if (geometry::OrientationOf3Vectors(cone.leftP - cone.p, p - cone.p, cone.rightP - cone.p) >= 0) return false;

	return true;
}

bool Intersect(const VV& s, const VV& t, const Segment& seg)
{
	if (s.p == seg.first || s.p == seg.second) return false;
	if (t.p == seg.first || t.p == seg.second) return false;

	if (Segment::SegmentSegmentIntersect(s.p, t.p, seg.first, seg.second)) return true;

	return false;
}
