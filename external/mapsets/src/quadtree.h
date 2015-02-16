#pragma once 

#include "common/geometry/point.h"
#include "common/geometry/rectangle.h"

const int QT_NODE_CAPACITY = 4;

class QuadTree
{
	Rectangle boundary;

	vector<Point> points;

	// Children
	QuadTree* northWest;
	QuadTree* northEast;
	QuadTree* southWest;
	QuadTree* southEast;

	// Methods
public:
	QuadTree(const Rectangle& boundary): boundary(boundary), northWest(NULL), northEast(NULL), southWest(NULL), southEast(NULL) {}

	double width() const
	{
		return boundary.xr - boundary.xl;
	}

	bool insert(const Point& p)
	{
		// Ignore objects which do not belong in this quad tree
		if (!boundary.Contains(p))
			return false; // object cannot be added

		// If there is space in this quad tree, add the object here
		if ((int)points.size() < QT_NODE_CAPACITY)
		{
			points.push_back(p);
			return true;
		}

		// Otherwise, we need to subdivide then add the point to whichever node will accept it
		if (northWest == NULL)
			subdivide();

		if (northWest->insert(p)) return true;
		if (northEast->insert(p)) return true;
		if (southWest->insert(p)) return true;
		if (southEast->insert(p)) return true;

		// Otherwise, the point cannot be inserted for some unknown reason (which should never happen)
		return false;
	}

	// create four children which fully divide this quad into four quads of equal area
	void subdivide()
	{
		northWest = new QuadTree(Rectangle(boundary.xl, boundary.midX(), boundary.midY(), boundary.yr));
		northEast = new QuadTree(Rectangle(boundary.midX(), boundary.xr, boundary.midY(), boundary.yr));
		southWest = new QuadTree(Rectangle(boundary.xl, boundary.midX(), boundary.yl, boundary.midY()));
		southEast = new QuadTree(Rectangle(boundary.midX(), boundary.xr, boundary.yl, boundary.midY()));
	}

	vector<Point> queryRange(const Rectangle& range)
	{
		// Prepare an array of results
		vector<Point> pointsInRange;

		// Automatically abort if the range does not collide with this quad
		if (!boundary.Intersects(range))
			return pointsInRange; // empty list

		// Check objects at this quad level
		for (int i = 0; i < (int)points.size(); i++)
		{
			if (range.Contains(points[i]))
				pointsInRange.push_back(points[i]);
		}

		// Terminate here, if there are no children
		if (northWest == NULL)
			return pointsInRange;

		// Otherwise, add the points from the children
		vector<Point> res;

		res = northWest->queryRange(range);
		pointsInRange.insert(pointsInRange.end(), res.begin(), res.end());
		res = northEast->queryRange(range);
		pointsInRange.insert(pointsInRange.end(), res.begin(), res.end());
		res = southWest->queryRange(range);
		pointsInRange.insert(pointsInRange.end(), res.begin(), res.end());
		res = southEast->queryRange(range);
		pointsInRange.insert(pointsInRange.end(), res.begin(), res.end());

		return pointsInRange;
	}

	bool containsInRange(const Rectangle& range)
	{
		// Automatically abort if the range does not collide with this quad
		if (!boundary.Intersects(range))
			return false; // empty list

		// Check objects at this quad level
		for (int i = 0; i < (int)points.size(); i++)
		{
			if (range.Contains(points[i]))
				return true;
		}

		// Terminate here, if there are no children
		if (northWest == NULL)
			return false;

		// Otherwise, add the points from the children
		if (northWest->containsInRange(range)) return true;
		if (northEast->containsInRange(range)) return true;
		if (southWest->containsInRange(range)) return true;
		if (southEast->containsInRange(range)) return true;

		return false;
	}

	bool containsInRange(const Rectangle& range, const Point& self)
	{
		// Automatically abort if the range does not collide with this quad
		if (!boundary.Intersects(range))
			return false; // empty list

		// Check objects at this quad level
		for (int i = 0; i < (int)points.size(); i++)
		{
			if (points[i] != self && range.Contains(points[i]))
				return true;
		}

		// Terminate here, if there are no children
		if (northWest == NULL)
			return false;

		// Otherwise, add the points from the children
		if (northWest->containsInRange(range, self)) return true;
		if (northEast->containsInRange(range, self)) return true;
		if (southWest->containsInRange(range, self)) return true;
		if (southEast->containsInRange(range, self)) return true;

		return false;
	}
};
