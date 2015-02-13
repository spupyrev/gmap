#pragma once
		
#include "common/common.h"
#include "common/random_utils.h"

#include <cmath>

class Point
{
public:
	double x, y;

	Point(): x(0), y(0) {}
	Point(double x, double y): x(x), y(y) {}
	Point(const Point& p): x(p.x), y(p.y) {}

	Point& operator = (const Point& p)
	{
		x = p.x;
		y = p.y;
		return *this;
	}

	bool operator < (const Point& p) const
	{
		return (Less(x, p.x) || (Equal(x, p.x) && Less(y, p.y)));
	}

	bool operator > (const Point& p) const
	{
		return (Greater(x, p.x) || (Equal(x, p.x) && Greater(y, p.y)));
	}

	bool operator == (const Point& p) const
	{
		return (Equal(x, p.x) && Equal(y, p.y));
	}

	bool operator != (const Point& p) const
	{
		return !(*this == p);
	}

	void Normalize()
	{
		double len = Distance(Point(0, 0));
		if (len > EPS)
		{
			x /= len;
			y /= len;
		}
	}

	double Distance(const Point& p) const
	{
		double ret = DistanceSquared(p);
		return sqrt(Abs(ret));
	}

	double DistanceSquared(const Point& p) const
	{
		return (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
	}

	double Length() const
	{
		return sqrt(Abs(LengthSquared()));
	}

	double LengthSquared() const
	{
		return Sqr2(x) + Sqr2(y);
	}

	void Scale(double s)
	{
		x *= s;
		y *= s;
	}

	double getCoordinate(int index)
	{
		if (index == 0) return x;
		else return y;
	}

	void setCoordinate(int index, double value)
	{
		if (index == 0) x = value;
		else y = value;
	}

	void operator *= (double s)
	{
		Scale(s);
	}

	void operator /= (double s)
	{
		if (Abs(s) > EPS)
		{
			Scale(1.0 / s);
		}
	}

	void operator += (const Point& p)
	{
		x += p.x;
		y += p.y;
	}

	void operator -= (const Point& p)
	{
		x -= p.x;
		y -= p.y;
	}

	Point operator + (const Point& p) const
	{
		Point ret = *this;
		ret += p;
		return ret;
	}

	Point operator - (const Point& p) const
	{
		Point ret = *this;
		ret -= p;
		return ret;
	}

	Point operator * (double s) const
	{
		Point ret = *this;
		ret.Scale(s);
		return ret;
	}

	Point operator / (double s) const
	{
		Point ret = *this;
		ret.Scale(1.0/s);
		return ret;
	}

	string toString() const
	{
		return "(" + to_string((int)x) + "," + to_string((int)y) + ")";
	}

	double Hash(double x)
	{
		double scale = 10;
		if (Abs(x) < EPS) return 0.0;
		return (log(x) + scale) / (2.0 * scale);
	}

    static double CrossProduct(Point point0, Point point1) 
	{
		return point0.x * point1.y - point0.y * point1.x;
	}

	static double DotProduct(Point point0, Point point1)
	{
		return point0.x * point1.x + point0.y * point1.y;
	}

    static Point RandomPoint(double minX, double maxX, double minY, double maxY) 
	{
		return Point(randDouble(minX, maxX), randDouble(minY, maxY));
    }

    static Point RandomPoint() 
	{
		return RandomPoint(0, 1, 0, 1);
    }
};
