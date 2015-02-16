#include "common/geometry/geometry_utils.h"

#include <set>

namespace geometry {

double getMinX(vector<Point>& points)
{
	double res = points[0].x;
	for (int i=1;i<(int)points.size();i++)
		res = min(res, points[i].x);
	return res;
}

double getMaxX(vector<Point>& points)
{
	double res = points[0].x;
	for (int i=1;i<(int)points.size();i++)
		res = max(res, points[i].x);
	return res;
}

double getMinY(vector<Point>& points)
{
	double res = points[0].y;
	for (int i=1;i<(int)points.size();i++)
		res = min(res, points[i].y);
	return res;
}

double getMaxY(vector<Point>& points)
{
	double res = points[0].y;
	for (int i=1;i<(int)points.size();i++)
		res = max(res, points[i].y);
	return res;
}

void CentralizePoints(vector<Point>& points)
{
	if (points.size() == 0)
		return;

	double mx = getMinX(points);
	double Mx = getMaxX(points);
	double my = getMinY(points);
	double My = getMaxY(points);

	double cenx = (mx + Mx) / 2.0;
	double ceny = (my + My) / 2.0;

	for (int i = 0; i < (int)points.size(); i++)
	{
		points[i].x -= cenx;
		points[i].y -= ceny;
	}
}

void ScalePoints(vector<Point>& points)
{
	if (points.size() == 0)
		return;

	double mx = getMinX(points);
	double Mx = getMaxX(points);
	double my = getMinY(points);
	double My = getMaxY(points);

	double scale = max(Abs(mx), Abs(Mx));
	scale = max(scale, Abs(my));
	scale = max(scale, Abs(My));

	scale /= 0.95;

	if (scale > EPS)
	{
		for (int i = 0; i < (int)points.size(); i++)
		{
			points[i].x /= scale;
			points[i].y /= scale;
		}
	}
}

vector<Segment> RectToSegments(const Rectangle& rect)
{
	vector<Segment> res;
	res.push_back(Segment(rect.xl, rect.yl, rect.xl, rect.yr));
	res.push_back(Segment(rect.xl, rect.yr, rect.xr, rect.yr));
	res.push_back(Segment(rect.xr, rect.yr, rect.xr, rect.yl));
	res.push_back(Segment(rect.xr, rect.yl, rect.xl, rect.yl));

	return res;
}

Point ClosestPoint(const Rectangle& rect, const Point& point)
{
	vector<Segment> rectSegs = RectToSegments(rect);

	Point res;
	double minDist = INF;
	for (int i = 0; i < (int)rectSegs.size(); i++)
	{
		Point closestPoint = ClosestPoint(rectSegs[i], point);
		double dist = closestPoint.Distance(point);
		if (dist < minDist)
		{
			minDist = dist;
			res = closestPoint;
		}
	}

	return res;
}

Point ClosestPoint(const Segment& seg, const Point& point)
{
	Point bc = seg.second - seg.first;
    Point ba = point - seg.first;
    double c1, c2;
	if ((c1 = Point::DotProduct(bc, ba)) <= 0.0 + EPS)
        return seg.first;

	if ((c2 = Point::DotProduct(bc, bc)) <= c1 + EPS)
        return seg.second;

    double parameter = c1 / c2;
    return seg.first + bc * parameter;
}

bool Intersect(const Rectangle& rect, const Segment& seg, Point& closestRectPoint, Point& closestSegPoint)
{
	double minDist = INF;

	vector<Segment> rectSegs = RectToSegments(rect);
	for (int i = 0; i < (int)rectSegs.size(); i++)
	{
		Point p1, p2;
		if (Segment::SegmentSegmentIntersect(rectSegs[i], seg, p1, p2)) return true;
		double dist = (p1 - p2).Length();
		if (minDist > dist)
		{
			minDist = dist;
			closestRectPoint = p1;
			closestSegPoint = p2;
		}
	}

	return false;
}

// computes orientation of three vectors with a common source
// (compare polar angles of v1 and v2 with respect to v0)
// <returns>
//  -1 if the clockwise orientation is v0 v1 v2
//   1 if the clockwise orientation is v0 v2 v1
//   0  if v1 and v2 are collinear and codirectinal
int OrientationOf3Vectors(const Point& vector0, const Point& vector1, const Point& vector2) 
{
    double xp2 = Point::CrossProduct(vector0, vector2);
	double dotp2 = Point::DotProduct(vector0, vector2);
    double xp1 = Point::CrossProduct(vector0, vector1);
	double dotp1 = Point::DotProduct(vector0, vector1);

    // v1 is collinear with v0
    if (Equal(xp1, 0.0) && GreaterOrEqual(dotp1, 0.0)) 
	{
        if (Equal(xp2, 0.0) && GreaterOrEqual(dotp2, 0.0))
            return 0;
        return 1;
    }

    // v2 is collinear with v0
    if (Equal(xp2, 0.0) && GreaterOrEqual(dotp2, 0.0)) 
	{
        return -1;
    }

    if (Equal(xp1, 0.0) || Equal(xp2, 0.0) || xp1 * xp2 > 0.0) 
	{
        // both on same side of v0, compare to each other
        return Compare(Point::CrossProduct(vector2, vector1), 0.0);
    }

    // vectors "less than" zero degrees are actually large, near 2 pi
    return -Compare(Sgn(xp1), 0.0);
}

// Find the Euclidean minimum spanning tree of the points
// returns a sequence of parent indices for every point (parent=-1 indicates the root)
vector<int> MinimumSpanningTree(const vector<Point>& points)
{
	int source = 0;

	VI used = VI(points.size(), 0);
	VI parent = VI(points.size(), -1);
	VD dist(points.size(), INF);
	dist[source] = 0;

	typedef pair<double, int> QE;
	set<QE> q;
	q.insert(make_pair(0.0, source));
 
	while (!q.empty())
	{
		//extract min
		int now = (*q.begin()).second;
		q.erase(q.begin());
		used[now] = 1;

		//update neighbors
		for (int i = 0; i < (int)points.size(); i++)
		{
			int next = i;
			if (used[next]) continue;

			double newDist = points[now].Distance(points[next]);

			if (dist[next] > newDist)
			{
				if (dist[next] != INF)
					q.erase(q.find(make_pair(dist[next], next)));

				dist[next] = newDist;
				parent[next] = now;
				q.insert(make_pair(dist[next], next));
			}
		}
	}

	return parent;
}

} // namespace geometry
