#pragma once
		
#include "common/geometry/point.h"

class Circle
{
    Point center;
    double squaredRadius;

public:
	Circle(): center(Point()), squaredRadius(0) {}
	Circle(const Point& center, double squaredRadius): center(center), squaredRadius(squaredRadius) {}

	inline Point getCenter() const
	{
		return center;
	}

    bool Contains(const Point& p) const
    {
		return (squaredRadius > center.DistanceSquared(p));
    }

};
