#pragma once
#pragma warning(disable:4996)

#include <string>
#include <cmath>

#include "point.h"
#include "rectangle.h"
using namespace std;

class Segment
{
	void init(const Point& x, const Point& y)
	{
		if (x < y) {first = x; second = y;}
		else {first = y; second = x;}
	}

public:
	Point first, second;

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
		return (first + second)*0.5;
	}

	double length() const
	{
		return second.Distance(first);
	}

	Rectangle boundingBox() const
	{
		double minX = min(first.x, second.x);
		double maxX = max(first.x, second.x);
		double minY = min(first.y, second.y);
		double maxY = max(first.y, second.y);

		return Rectangle(minX, maxX, minY, maxY);
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
