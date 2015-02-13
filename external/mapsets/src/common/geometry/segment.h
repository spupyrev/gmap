#pragma once

#include "common/geometry/point.h"

class Segment
{
	void init(const Point& x, const Point& y)
	{
		if (x < y) {first = x; second = y;}
		else {first = y; second = x;}
	}

public:
	Point first;
	Point second;

	Segment(): first(), second() {}
	Segment(const Point& x, const Point& y) 
	{
		init(x, y);
	}

	Segment(double x1, double y1, double x2, double y2)
	{
		init(Point(x1, y1), Point(x2, y2));
	}

	bool operator == (const Segment& o) const
	{
		return (first == o.first && second == o.second) || (first == o.second && second == o.first);
	}

	bool operator != (const Segment& o) const
	{
		return !(*this == o);
	}

	bool operator < (const Segment& o) const
	{
		if (first < o.first) return true;
		if (first == o.first && second < o.second) return true;

		return false;
	}

	Point middle() const
	{
		return (first + second) * 0.5;
	}

	double length() const
	{
		return second.Distance(first);
	}

	static bool SegmentSegmentIntersect(const Segment& seg1, const Segment& seg2, Point& closestPointOnSeg1, Point& closestPointOnSeg2)
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

	static bool SegmentSegmentIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
	{
		//look for the solution in the form a+u*(b-a)=c+v*(d-c)
		Point ba = b - a;
		Point cd = c - d;
		Point ca = c - a;

		double u, v;
		bool ret = SolveLinearSystem(ba.x, cd.x, ca.x, ba.y, cd.y, ca.y, u, v);
		if (ret && u > -EPS && u < 1.0 + EPS && v > -EPS && v < 1.0 + EPS)
			return true;
		else
			return false;
	}

	static bool SegmentSegmentIntersect(const Segment& seg1, const Segment& seg2)
	{
		return SegmentSegmentIntersect(seg1.first, seg1.second, seg2.first, seg2.second);
	}

	//intersections at endpoints are not counted
	static bool EdgesIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
	{
		if (a == c || a == d || b == c || b == d) return false;
		return SegmentSegmentIntersect(a, b, c, d);
	}

	//intersections at endpoints are not counted
	static bool EdgesIntersect(const Segment& seg1, const Segment& seg2)
	{
		return EdgesIntersect(seg1.first, seg1.second, seg2.first, seg2.second);
	}

	//intersections at endpoints are not counted
	static bool EdgesIntersect(const Segment& s, const vector<Segment>& segs)
	{
		for (int i = 0; i < (int)segs.size(); i++)
			if (EdgesIntersect(s.first, s.second, segs[i].first, segs[i].second)) return true;

		return false;
	}

	//intersections at endpoints are not counted
	static double CrossingAngle(const Point& a, const Point& b, const Point& c, const Point& d)
	{
		const double PI = 3.14159265358979323846;
		const double EPS = 1e-6;

		Point ba = b - a;
		Point cd = c - d;

		double cross = Point::CrossProduct(ba, cd);
		double dot = Point::DotProduct(ba, cd);

		if (Abs(dot) < EPS)
		{
			if (Abs(cross) < EPS)
				return 0;

			return PI / 2;
		}

		if (Abs(cross) < EPS)
		{
			return 0.0;
		}

		double at = atan2(cross, dot);
		if (cross >= -EPS)
			return min(at, PI - at);

		return min(PI + at, -at);
	}

	static bool SolveLinearSystem(double a00, double a01, double b0, double a10, double a11, double b1, double& x, double& y)
	{
		const double EPS = 1e-6;

		double d = a00 * a11 - a10 * a01;

		if (abs(d) < EPS)
		{
			x = y = 0; //to avoid the compiler bug
			return false;
		}

		x = (b0 * a11 - b1 * a01) / d;
		y = (a00 * b1 - a10 * b0) / d;

		return true;
	}
};
