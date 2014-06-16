#include "closest_point.h"
#include "time_utils.h"
#include "segment.h"
#include "rectangle_node.h"

void TestClosestPoints()
{
	int N = 100000;
	vector<Point> points;
	for (int i = 0; i < N; i++)
	{
		Point p = Point::RandomPoint(-100, 100, -100, 100);
		points.push_back(p);
	}

	int Q = 100000;
	vector<Point> query;
	for (int i = 0; i < Q; i++)
	{
		Point p = Point::RandomPoint(-100, 100, -100, 100);
		query.push_back(p);
	}

	OutputTimeInfo("starting test");

	ClosestPointQP cp(Rectangle(-101, 101, -101, 101));
	//ClosestPoint cp;
	for (int i = 0; i < (int)points.size(); i++)
		cp.addPoint(points[i]);

	OutputTimeInfo("%d points added", points.size());

	double sum = 0;
	for (int i = 0; i < (int)query.size(); i++)
	{
		Point p = cp.getClosest(query[i]);
		sum += p.Distance(query[i]);
		//OutputTimeInfo("done %d", i);
	}

	OutputTimeInfo("test done: %.3lf", sum);
}

bool Intersect(const Segment& seg, const vector<Segment>& obstacles)
{
	//check crossings with obstacles
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		if (Segment::SegmentSegmentIntersect(seg.first, seg.second, obstacles[i].first, obstacles[i].second)) return true;
	}

	return false;
}

/// Computes the intersection between the hub and obstacles
int TreeCount(RectangleNode* node)
{
	if (!node->IsLeaf()) 
	{
		int cnt = TreeCount(node->getLeft());
		cnt += TreeCount(node->getRight());

		return cnt;
    }

	return 1;
}

bool IntersectSegmentWithTree(const Segment& seg, RectangleNode* obstacleTreeRoot)
{
	vector<RectangleNode*> touchedObstacles;
	IntersectRectangleWithTree(obstacleTreeRoot, seg.boundingBox(), touchedObstacles);
	//OutputTimeInfo("testing %d obstacles", touchedObstacles.size());

	for (int i = 0; i < (int)touchedObstacles.size(); i++)
	{
		Segment obstacle = touchedObstacles[i]->getData();
		if (Segment::SegmentSegmentIntersect(seg.first, seg.second, obstacle.first, obstacle.second)) return true;
	}

	return false;
}

void TestSegmentIntersections()
{
	int N = 100000;
	vector<Segment> segments;
	for (int i = 0; i < N; i++)
	{
		Point p = Point::RandomPoint(-100, 100, -100, 100);
		Point q = Point::RandomPoint(-5, 5, -5, 5);
		//Point q = Point::RandomPoint(-100, 100, -100, 100);
		segments.push_back(Segment(p, p+q));
	}

	int Q = 100000;
	vector<Segment> query;
	for (int i = 0; i < Q; i++)
	{
		Point p = Point::RandomPoint(-100, 100, -100, 100);
		Point q = Point::RandomPoint(-10, 10, -10, 10);
		query.push_back(Segment(p, p+q));
	}

	OutputTimeInfo("starting test");

	vector<RectangleNode*> nodes;
	for (int i = 0; i < (int)segments.size(); i++)
	{
		RectangleNode* node = new RectangleNode(segments[i].boundingBox(), segments[i]);
		nodes.push_back(node);
	}

	RectangleNode* obstacleTreeRoot = RectangleNode::CreateRectangleNodeOnEnumeration(nodes);
	assert(obstacleTreeRoot != NULL);
	OutputTimeInfo("tree with %d nodes constructed", TreeCount(obstacleTreeRoot));

	double sum = 0;
	double all = 0;
	for (int i = 0; i < (int)query.size(); i++)
	{
		double len = query[i].length();
		all += len;

		//if (Intersect(query[i], segments))
		if (IntersectSegmentWithTree(query[i], obstacleTreeRoot))
			sum += len;
	}

	OutputTimeInfo("test done: %.3lf  (all: %.3lf)", sum, all);
}
