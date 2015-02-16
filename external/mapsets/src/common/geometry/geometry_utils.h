#pragma once

#include "common/geometry/point.h"
#include "common/geometry/segment.h"
#include "common/geometry/rectangle.h"

namespace geometry {

double getMinX(vector<Point>& points);
double getMaxX(vector<Point>& points);
double getMinY(vector<Point>& points);
double getMaxY(vector<Point>& points);

// Centering points around origin
void CentralizePoints(vector<Point>& points);

// Scaling points to fit into the unit square
void ScalePoints(vector<Point>& points);

// The closest point on the given "rect" to the given "point". 
Point ClosestPoint(const Rectangle& rect, const Point& point);

// The closest point on "segment" to "point".
Point ClosestPoint(const Segment& segment, const Point& point);

// Does the given rectangle intersect the line segment?
bool Intersect(const Rectangle& rect, const Segment& seg, Point& closestRectPoint, Point& closestSegPoint);

// computes orientation of three vectors with a common source
// (compare polar angles of v1 and v2 with respect to v0)
//
//  -1 if the clockwise orientation is v0 v1 v2
//   1 if the clockwise orientation is v0 v2 v1
//   0  if v1 and v2 are collinear and codirectinal
int OrientationOf3Vectors(const Point& vector0, const Point& vector1, const Point& vector2);

// Find a convex hull of the given points
// returns a sequence of indices of points
vector<int> ConvexHull(const vector<Point>& points);

// Find the Euclidean minimum spanning tree of the points
// returns a sequence of parent indices for every point (parent=-1 indicates the root)
vector<int> MinimumSpanningTree(const vector<Point>& points);

} // namespace geometry