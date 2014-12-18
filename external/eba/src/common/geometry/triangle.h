#pragma once

#include <cassert>

#include "common/geometry/point.h"
#include "common/geometry/circle.h"

namespace geometry {

enum PointSegmentRelation {ONSEGMENT, LEFT, RIGHT, INFRONTOFA, BEHINDB, ERROR};

class Triangle
{
public:
	Point a, b, c;

	// triangles that share the corresponding side with this triangle
	Triangle* abnext;
	Triangle* bcnext;
	Triangle* canext;

	// true iff it is an infinite face
    bool halfplane; 

	// circumcircle around the triangle
    Circle circumcircle;

public:
    // creates a half plane using segment (A,B)
    Triangle(const Point& A, const Point& B): a(A), b(B), halfplane(true)
    {
    }

    // creates a triangle sorting points in counterclockwise order
	Triangle(const Point& A, const Point& B, const Point& C): halfplane(false)
	{
		a = A;
		int res = PointLineTest(C, A, B);
		if ((res <= LEFT) || (res == INFRONTOFA) || (res == BEHINDB))
		{
			b = B;
			c = C;
		}
		else
		{ // RIGHT
			b = C;
			c = B;
		}

        InitCircumcircle();
	} 

	~Triangle()
	{
	}

    // Checks if the given point is a corner of this triangle
    bool isCorner(const Point& p) const
    {
        return ((p == a) || (p == b) || (!halfplane && p == c));
    }

    void InitCircumcircle()
    {
        double u = ((a.x - b.x) * (a.x + b.x) + (a.y - b.y) * (a.y + b.y)) / 2.0;
        double v = ((b.x - c.x) * (b.x + c.x) + (b.y - c.y) * (b.y + c.y)) / 2.0;
        double den = (a.x - b.x) * (b.y - c.y) - (b.x - c.x) * (a.y - b.y);

        if (Abs(den) < EPS) // degenerate case
		{
            circumcircle = Circle(a, INF);
		}
        else
        {
			double xCenter = (u * (b.y - c.y) - v * (a.y - b.y)) / den;
			double yCenter = (v * (a.x - b.x) - u * (b.x - c.x)) / den;
            Point center(xCenter, yCenter);
            circumcircle = Circle(center, center.DistanceSquared(a));
        }
    }

	// determinates if this triangle contains the point p (boundary is considered inside)
	bool Contains(const Point& p) const
	{
		bool ans = false;
		if (halfplane) 
			return false;

		if (isCorner(p))
			return true;

		int a12 = PointLineTest(p, a, b);
		int a23 = PointLineTest(p, b, c);
		int a31 = PointLineTest(p, c, a);

		if ((a12 == LEFT && a23 == LEFT && a31 == LEFT)
				|| (a12 == RIGHT && a23 == RIGHT && a31 == RIGHT)
				|| (a12 == ONSEGMENT || a23 == ONSEGMENT || a31 == ONSEGMENT))
			ans = true;

		return ans;
	}

	// determinates if the boundary of the triangle contains the point p
	bool OnBoundary(const Point& p) const
	{
		if (halfplane)
			return false;

		if (isCorner(p))
			return true;

		return (PointLineTest(p, a, b) == ONSEGMENT || PointLineTest(p, b, c) == ONSEGMENT || PointLineTest(p, a, c) == ONSEGMENT);
	}

	bool InsideCircumcircle(const vector<Point>& points) const
	{
		for (const Point& p : points)
		{
			if (!isCorner(p) && circumcircle.Contains(p))
				return true;
		}

		return false;
	}

	void SwitchNeighbors(Triangle* oldNeighbor, Triangle* newNeighbor)
	{
		if (abnext == oldNeighbor)
			abnext = newNeighbor;
		else if (bcnext == oldNeighbor)
			bcnext = newNeighbor;
		else if (canext == oldNeighbor)
			canext = newNeighbor;
		else
			assert(false);
	}

	// Returns the neighbors that shares the given corner and is different from the given triangle
	Triangle* NextNeighbor(const Point& corner, Triangle* prevTriangle)
	{
		Triangle* neighbor = nullptr;

		if (a == corner)
		{
			neighbor = canext;
		}
		if (b == corner)
		{
			neighbor = abnext;
		}
		if (c == corner)
		{
			neighbor = bcnext;
		}

		// If the current neighbor is a half plane, we also want to move to the next neighbor	  
		if (neighbor == prevTriangle)// || neighbor->halfplane)
		{
			if (a == corner)
			{
				neighbor = abnext;
			}
			if (b == corner)
			{
				neighbor = bcnext;
			}
			if (c == corner)
			{
				neighbor = canext;
			}
		}

		return neighbor;
	}

    // checks if the 2 triangles shares a side
    bool ShareSide(const Triangle* t2)
	{
		if (isCorner(t2->a) && isCorner(t2->b))
			return true;

		if (!t2->halfplane && isCorner(t2->a) && isCorner(t2->c))
			return true;

		if (!t2->halfplane && isCorner(t2->b) && isCorner(t2->c))
			return true;

		return false;
	}

	//checks if the triangle is not re-entrant
	static bool isConvex(const Point& a, const Point& b, const Point& c)
	{
		double det = (a.x * (b.y - c.y)) - (a.y * (b.x - c.x)) + (b.x * c.y - b.y * c.x);
		return (det >= 0.0);
	}

	/**
		* tests the relation between this point (as a 2D [x,y] point) and a 2D
		* segment a,b (the Z values are ignored), returns one of the following:
		* LEFT, RIGHT, INFRONTOFA, BEHINDB, ONSEGMENT
		*/
	static int PointLineTest(const Point& self, const Point& a, const Point& b)
	{
		double dx = b.x - a.x;
		double dy = b.y - a.y;
		double res = dy * (self.x - a.x) - dx * (self.y - a.y);

		if (Less(res, 0))
			return LEFT;
		if (Greater(res, 0))
			return RIGHT;

		if (Greater(dx, 0))
		{
			if (Less(self.x, a.x))
				return INFRONTOFA;
			if (Less(b.x, self.x))
				return BEHINDB;
			return ONSEGMENT;
		}
		if (Less(dx, 0))
		{
			if (Greater(self.x, a.x))
				return INFRONTOFA;
			if (Greater(b.x, self.x))
				return BEHINDB;
			return ONSEGMENT;
		}
		if (Greater(dy, 0))
		{
			if (Less(self.y, a.y))
				return INFRONTOFA;
			if (Less(b.y, self.y))
				return BEHINDB;
			return ONSEGMENT;
		}
		if (Less(dy, 0))
		{
			if (Greater(self.y, a.y))
				return INFRONTOFA;
			if (Greater(b.y, self.y))
				return BEHINDB;
			return ONSEGMENT;
		}

		assert(false);
		//cerr << "Error, pointLineTest with a=b" << endl;
		return ERROR;
	}
};

} // namespace geometry
