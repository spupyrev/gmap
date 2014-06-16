#pragma once

#include "common.h"
#include "random_utils.h"

class Point
{
	double F(double x)
	{
		double scale = 10;
		if ( fabs(x)<1e-6 ) return 0.0;
		return (log(x) + scale)/(2.0*scale);
	}

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
		return (x < p.x || (x == p.x && y < p.y));
	}

	bool operator == (const Point& p) const
	{
		return (fabs(x - p.x) < 1e-8 && fabs(y - p.y) < 1e-8);
	}

	bool operator != (const Point& p) const
	{
		return !(*this == p);
	}

	bool Read()
	{
		return ( scanf("%lf %lf", &x, &y)==2 );
	}

	void Normalize()
	{
		double len = Distance(Point(0, 0));
		if (len > 1e-6)
		{
			x /= len;
			y /= len;
		}
	}

	double Distance(const Point& p) const
	{
		double ret = Distance2(p);
		return sqrt(fabs(ret));
	}

	double Length() const
	{
		return sqrt(fabs(Sqr2(x) + Sqr2(y)));
	}

	double LengthSquared() const
	{
		return Sqr2(x) + Sqr2(y);
	}

	double Distance2(const Point& p) const
	{
		double ret = Sqr2(x-p.x) + Sqr2(y-p.y);
		return ret;
	}

	void Scale(double s)
	{
		x *= s;
		y *= s;
	}

	double getCoordinate(int index)
	{
		if ( index==0 ) return x;
		else return y;
	}

	void setCoordinate(int index, double value)
	{
		if ( index==0 ) x = value;
		else y = value;
	}

	void operator *= (double s)
	{
		Scale(s);
	}

	void operator /= (double s)
	{
		if (fabs(s) > 1e-8)
		{
			Scale(1.0/s);
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
