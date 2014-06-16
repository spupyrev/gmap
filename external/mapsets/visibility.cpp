#include "visibility.h"
#include "comparer.h"
#include "graph_algorithms.h"
#include "rectangle_node.h"
#include "time_utils.h"
#include "closest_point.h"

#include <map>
#include <algorithm>

typedef vector<Point> VP;

vector<VV> CreateVisibilityVertices(const vector<Point>& p, const vector<Segment>& obstacles);
vector<vector<int> > CreateVisibilityEdges(const vector<VV>& vis, const vector<Segment>& obstacles);
bool IsVisible(const VV& s, const VV& t, const vector<Segment>& obstacles, RectangleNode* obstacleTreeRoot);
bool IsInCone(const VV& cone, const Point& p);
int GetOrientationOf3Vectors(Point vector0, Point vector1, Point vector2);
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
		return GetOrientationOf3Vectors(head - origin, p1 - origin, p2 - origin) <= 0;
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

	vector<RectangleNode*> nodes;
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		RectangleNode* node = new RectangleNode(obstacles[i].boundingBox(), obstacles[i]);
		nodes.push_back(node);
	}

	RectangleNode* obstacleTreeRoot = RectangleNode::CreateRectangleNodeOnEnumeration(nodes);
	assert(obstacleTreeRoot != NULL);

	vector<vector<int> > edges = VVI(vis.size(), VI());

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
		vector<Point> adjP = cp.getKClosest(vis[i].p, maxAdj, 32);
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
	
	//full visibility
	/*for (int i = 0; i < (int)vis.size(); i++)
	{
		for (int j = 0; j < (int)vis.size(); j++)
			if (i != j)
			{
				if (IsVisible(vis[i], vis[j], obstacles, obstacleTreeRoot)) edges[i].push_back(j);
			}
	} */
	
	//OutputTimeInfo("edges computed");
	return edges;
}

bool IntersectRectangleWithTree2(RectangleNode* node, const Rectangle& rect, const VV& s, const VV& t)
{
	if (!node->getRectangle().intersects(rect)) return false;

	if (!node->IsLeaf()) 
	{
		return IntersectRectangleWithTree2(node->getLeft(), rect, s, t) || IntersectRectangleWithTree2(node->getRight(), rect, s, t);
    }
    else 
	{
		if (Intersect(s, t, node->getData())) return true;
    }

	return false;
}

bool IntersectSegmentWithTree(const VV& s, const VV& t, RectangleNode* obstacleTreeRoot)
{
	Rectangle bb(s.p);
	bb.Add(t.p);
	return IntersectRectangleWithTree2(obstacleTreeRoot, bb, s, t);

	vector<RectangleNode*> touchedObstacles;
	for (int i = 0; i < (int)touchedObstacles.size(); i++)
	{
		Segment obstacle = touchedObstacles[i]->getData();
		if (Intersect(s, t, obstacle)) return true;
	}

	return false;
}

bool IntersectSegment(const VV& s, const VV& t, const vector<Segment>& obstacles)
{
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		if (Intersect(s, t, obstacles[i])) return true;
	}

	return false;
}

bool IsVisible(const VV& s, const VV& t, const vector<Segment>& obstacles, RectangleNode* obstacleTreeRoot)
{
	if (s.p == t.p) return false;

	//check crossings with obstacles
	//if (IntersectSegment(s, t, obstacles)) return false;
	if (IntersectSegmentWithTree(s, t, obstacleTreeRoot)) return false;

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

	if (GetOrientationOf3Vectors(cone.leftP - cone.p, p - cone.p, cone.rightP - cone.p) >= 0) return false;

	return true;
}

bool Intersect(const VV& s, const VV& t, const Segment& seg)
{
	if (s.p == seg.first || s.p == seg.second) return false;
	if (t.p == seg.first || t.p == seg.second) return false;

	if (Segment::SegmentSegmentIntersect(s.p, t.p, seg.first, seg.second)) return true;

	return false;
}

// computes orientation of three vectors with a common source
// (compare polar angles of v1 and v2 with respect to v0)
// <returns>
//  -1 if the orientation is v0 v1 v2
//   1 if the orientation is v0 v2 v1
//   0  if v1 and v2 are collinear and codirectinal
int GetOrientationOf3Vectors(Point vector0, Point vector1, Point vector2) 
{
    double xp2 = Point::CrossProduct(vector0, vector2);
	double dotp2 = Point::DotProduct(vector0, vector2);
    double xp1 = Point::CrossProduct(vector0, vector1);
	double dotp1 = Point::DotProduct(vector0, vector1);

    // v1 is collinear with v0
    if (Close(xp1, 0.0) && GreaterOrEqual(dotp1, 0.0)) 
	{
        if (Close(xp2, 0.0) && GreaterOrEqual(dotp2, 0.0))
            return 0;
        return 1;
    }

    // v2 is collinear with v0
    if (Close(xp2, 0.0) && GreaterOrEqual(dotp2, 0.0)) 
	{
        return -1;
    }

    if (Close(xp1, 0.0) || Close(xp2, 0.0) || xp1 * xp2 > 0.0) 
	{
        // both on same side of v0, compare to each other
        return Compare(Point::CrossProduct(vector2, vector1), 0.0);
    }

    // vectors "less than" zero degrees are actually large, near 2 pi
    return -Compare(Sgn(xp1), 0.0);
}
