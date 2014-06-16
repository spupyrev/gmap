#include "geom_utils.h"

bool SegmentSegmentIntersect(const Segment& seg1, const Segment& seg2, Point& closestPointOnSeg1, Point& closestPointOnSeg2)
{
	Point a = seg1.first;
	Point b = seg1.second;
	Point c = seg2.first;
	Point d = seg2.second;

	//look for the solution in the form a+u*(b-a)=c+v*(d-c)
    Point u = b - a;
    Point v = d - c;
    Point w = a - c;

    double D = Point::CrossProduct(u, v);

	double uu = Point::DotProduct(u, u);        // always >= 0
    double uv = Point::DotProduct(u, v);
    double vv = Point::DotProduct(v, v);        // always >= 0
    double uw = Point::DotProduct(u, w);
    double vw = Point::DotProduct(v, w);
    double sN, tN;
    double sD = D, tD = D;

    // compute the line parameters of the two closest points
    if (Abs(D) < EPS) 
	{ // the lines are almost parallel
        sN = 0.0;        // force using point a on segment [a..b]
        sD = 1.0;        // to prevent possible division by 0.0 later
        tN = vw;
        tD = vv;
    }
    else 
	{                // get the closest points on the infinite lines
        sN = Point::CrossProduct(v, w);
        tN = Point::CrossProduct(u, w);
        if (D < 0) 
		{
            sN = -sN;
            tN = -tN;
        }

        if (sN < 0.0) 
		{       // parab < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = vw;
            tD = vv;
        }
        else if (sN > sD) 
		{  // parab > 1 => the s=1 edge is visible
            sN = sD = 1;
            tN = vw + uv;
            tD = vv;
        }
    }

    if (tN < 0.0) 
	{           // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute parab for this edge
        if (-uw < 0.0)
            sN = 0.0;
        else if (-uw > uu)
            sN = sD;
        else 
		{
            sN = -uw;
            sD = uu;
        }
    }
    else if (tN > tD) 
	{      // tc > 1 => the t=1 edge is visible
        tN = tD = 1;
        // recompute parab for this edge
        if ((-uw + uv) < 0.0)
            sN = 0;
        else if ((-uw + uv) > uu)
            sN = sD;
        else 
		{
            sN = (-uw + uv);
            sD = uu;
        }
    }

    // finally do the division to get parameters
    double parab = (Abs(sN) < EPS ? 0.0 : sN / sD);
    double parcd = (Abs(tN) < EPS ? 0.0 : tN / tD);

	closestPointOnSeg1 = u*parab + a;
	closestPointOnSeg2 = v*parcd + c;

	return (closestPointOnSeg1.Distance(closestPointOnSeg2) < EPS);
}

/// The closest point on the segment [segmentStart,segmentEnd] to "point". 
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

/// The closest point on the segment [segmentStart,segmentEnd] to "point". 
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

/// Does the given rectangle intersect the line segment?
bool Intersect(const Rectangle& rect, const Segment& seg, Point& closestRectPoint, Point& closestSegPoint)
{
	double minDist = INF;

	vector<Segment> rectSegs = RectToSegments(rect);
	for (int i = 0; i < (int)rectSegs.size(); i++)
	{
		Point p1, p2;
		if (SegmentSegmentIntersect(rectSegs[i], seg, p1, p2)) return true;
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

vector<Segment> RectToSegments(const Rectangle& rect)
{
	vector<Segment> res;
	res.push_back(Segment(rect.xl, rect.yl, rect.xl, rect.yr));
	res.push_back(Segment(rect.xl, rect.yr, rect.xr, rect.yr));
	res.push_back(Segment(rect.xr, rect.yr, rect.xr, rect.yl));
	res.push_back(Segment(rect.xr, rect.yl, rect.xl, rect.yl));

	return res;
}

