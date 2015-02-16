#pragma once 

#include "common/common.h"
#include "common/geometry/point.h"
#include "common/geometry/rectangle.h"

#include "quadtree.h"

#include <algorithm>

class ClosestPoint
{
	vector<Point> points;

public:
	void addPoint(const Point& p)
	{
		points.push_back(p);
	}

	Point getClosest(const Point& p) const
	{
		double bestDist2 = -1;
		int bestIndex = -1;
		for (int i = 0; i < (int)points.size(); i++)
		{
			double dist2 = p.DistanceSquared(points[i]);
			if (bestIndex == -1 || dist2 < bestDist2)
			{
				bestIndex = i;
				bestDist2 = dist2;
			}
		}

		assert(bestIndex != -1);
		return points[bestIndex];
	}
};

class ClosestPointQP
{
	QuadTree* quadtree;

public:
	ClosestPointQP(const Rectangle& boundingBox) 
	{
		quadtree = new QuadTree(boundingBox);
	}

	void addPoint(const Point& p)
	{
		assert(quadtree->insert(p));
	}

	Point getClosest(const Point& p) const
	{
		double EPS = 1e-1;
		double l = EPS, r = EPS;
		while (true)
		{
			Rectangle range(p.x - r, p.x + r, p.y - r, p.y + r);
			if (quadtree->containsInRange(range)) break;
			else r *= 2.0;
		}

		l = r/2.0;
		while (Abs(r - l) > EPS)
		{
			double c = (l + r)/2.0;
			Rectangle range(p.x - c, p.x + c, p.y - c, p.y + c);
			if (quadtree->containsInRange(range)) r = c;
			else l = c;
		}

		double K = 1.42;
		Rectangle range(p.x - K*r, p.x + K*r, p.y - K*r, p.y + K*r);
		vector<Point> closest = quadtree->queryRange(range);
		assert(!closest.empty());

		Point best = closest[0];
		double bestLen = best.Distance(p);
		for (int i = 1; i < (int)closest.size(); i++)
		{
			double len = closest[i].Distance(p);
			if (len < bestLen)
			{
				bestLen = len;
				best = closest[i];
			}
		}

		return best;
	}

	vector<Point> getKClosest(const Point& p, int K) const
	{
		return getKClosest(p, K, 8.0);
	}

	vector<Point> getKClosest(const Point& p, int K, double D) const
	{
		double EPS = 1e-1;
		double r = EPS;
		//find min radius containing a point
		while (true)
		{
			Rectangle range(p.x - r, p.x + r, p.y - r, p.y + r);
			if (quadtree->containsInRange(range, p)) break;
			else r *= 2.0;
		}
		r /= 2.0;

		Rectangle range(p.x - D*r, p.x + D*r, p.y - D*r, p.y + D*r);
		vector<Point> closest = quadtree->queryRange(range);
		assert(!closest.empty());

		vector<pair<double, Point> > distP;
		for (int i = 0; i < (int)closest.size(); i++)
		{
			if (closest[i] == p) continue;
			double len = closest[i].Distance(p);
			distP.push_back(make_pair(len, closest[i]));
		}

		sort(distP.begin(), distP.end());
		vector<Point> res;
		for (int i = 0; i < (int)distP.size() && i < K; i++)
			res.push_back(distP[i].second);

		return res;
	}
};
