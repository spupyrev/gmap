#pragma once 

#include "segment.h"
#include "rectangle.h"

/// The closest point on the segment [segmentStart,segmentEnd] to "point". 
Point ClosestPoint(const Rectangle& rect, const Point& point);

/// The closest point on the segment [segmentStart,segmentEnd] to "point". 
Point ClosestPoint(const Segment& seg, const Point& point);

/// Does the given rectangle intersect the line segment?
bool Intersect(const Rectangle& rect, const Segment& seg, Point& closestRectPoint, Point& closestSegPoint);

vector<Segment> RectToSegments(const Rectangle& rect);