#pragma once

#include "common/geometry/point.h"

class Rectangle
{
public:
	double xl, xr;
	double yl, yr;

	Rectangle() {}
	Rectangle(const Point& p): xl(p.x), xr(p.x), yl(p.y), yr(p.y) {}
	Rectangle(double xl, double xr, double yl, double yr): xl(xl), xr(xr), yl(yl), yr(yr) {}

	Rectangle(const Rectangle& rect0, const Rectangle& rect1)
	{
		xl = rect0.xl;
		xr = rect0.xr;
		yl = rect0.yl;
		yr = rect0.yr;

		Add(rect1.xl, rect1.yl);
		Add(rect1.xr, rect1.yr);
	}

	Point minPoint() const
	{
		return Point(xl, yl);
	}

	Point maxPoint() const
	{
		return Point(xr, yr);
	}

	double minX() const
	{
		return xl;
	}

	double minY() const
	{
		return yl;
	}

	double midX() const
	{
		return (xl + xr) / 2.0;
	}

	double midY() const
	{
		return (yl + yr) / 2.0;
	}

	double area() const
	{
		return (xr - xl) * (yr - yl);
	}

	double getWidth() const
	{
		return (xr - xl);
	}

	double getHeight() const
	{
		return (yr - yl);
	}

	void Add(const Point& p)
	{
		Add(p.x, p.y);
	}

	void Add(const Rectangle& r)
	{
		Add(r.minPoint());
		Add(r.maxPoint());
	}

	void Add(double x, double y)
	{
		xl = min(xl, x);
		xr = max(xr, x);
		yl = min(yl, y);
		yr = max(yr, y);
	}

	bool Contains(const Point& p) const
	{
		return ((xl <= p.x && p.x <= xr) && (yl <= p.y && p.y <= yr));
	}

	bool Contains(const Rectangle& r) const
	{
		return ((xl <= r.xl && r.xr <= xr) && (yl <= r.yl && r.yr <= yr));
	}

	bool Intersects(const Rectangle& p) const
	{
		if (xr < p.xl || p.xr < xl) return false;
		if (yr < p.yl || p.yr < yl) return false;
		return true;
	}

};
