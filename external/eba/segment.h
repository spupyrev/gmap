#pragma once
#pragma warning(disable:4996)

#include <string>
#include <cmath>

#include "point.h"
using namespace std;

class Segment
{
public:
	Point first, second;

	Segment(): first(), second() {}
	Segment(Point x, Point y): first(x), second(y) {}

	bool operator = (const Segment& o) const
	{
		return (first == o.first && second == o.second) || (first == o.second && second == o.first);
	}

	bool operator < (const Segment& o) const
	{
		return length() < o.length();
	}

	Point middle() const
	{
		return (first + second)*0.5;
	}

	double length() const
	{
		return second.Distance(first);
	}

	static bool SegmentSegmentIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
	{
		const double EPS = 1e-6;

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

	//intersections at endpoints are not counted
	static bool EdgesIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
	{
		if (a == c || a == d || b == c || b == d) return false;
		return SegmentSegmentIntersect(a, b, c, d);
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
