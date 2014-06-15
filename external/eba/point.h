#pragma once
#pragma warning(disable:4996)
#ifndef _POINT_H_
#define _POINT_H_

#include <string>
#include <cmath>
#include <cstdio>
using namespace std;

#define Sqr(a) ((a)*(a))

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

	bool operator < (const Point& p) const
	{
		return (x<p.x || (x==p.x && y<p.y));
	}

	bool Read()
	{
		return ( scanf("%lf %lf", &x, &y)==2 );
	}

	double Distance(const Point& p) const
	{
		double ret = Sqr(x-p.x) + Sqr(y-p.y);
		return sqrt(fabs(ret));
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
		if ( fabs(s)>1e-8 )
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
};

class Rectangle
{
public:
	//left-bottom corner
	Point pLB;

	//right-top corner
	Point pRT;

	Rectangle(): pLB(0, 0), pRT(0, 0) {}
	Rectangle(Point pLB, Point pRT): pLB(pLB), pRT(pRT) {}
	Rectangle(double xl, double yl, double xr, double yr): pLB(xl, yl), pRT(xr, yr) {}

	double getWidth() const
	{
		return pRT.x - pLB.x;
	}

	double getHeight() const
	{
		return pRT.y - pLB.y;
	}

	bool contains(const Point& p) const
	{
		return ((pLB.x <= p.x && p.x <= pRT.x) && (pLB.y <= p.y && p.y <= pRT.y));
	}
};

#endif
