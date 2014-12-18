#pragma once

#include <set>
#include <memory>

#include "common/geometry/point.h"
#include "common/geometry/segment.h"
#include "common/geometry/rectangle.h"

#include "triangle.h"
#include "grid_index.h"

namespace geometry {

class GridIndex;

/**
 * This class represents a Delaunay Triangulation. The class was written for a
 * large scale triangulation (1000 - 200,000 vertices).

 * The class main properties are the following:
 * - fast point location. (O(n^0.5)), practical runtime is often very fast
 * - handles degenerate cases and none general position input (ignores duplicate points)
 */
class DelaunayTriangulation
{
	friend class GridIndex;

private:
    // the first and last points (used only for the first step construction)
    Point firstP;
    Point lastP;

    // the first and last triangles (used only for the first step construction)
    Triangle* firstT;
	Triangle* lastT;

    // for degenerate case
    bool allPointsCollinear;

    // the triangle the fond (search starts from here)
    Triangle* startTriangle;

    // the set of all (distinct) points in the triangulation
    set<Point> vertices;

    // the Bounding Box
	Rectangle* boundingBox;

    // Index for faster point location searches
    mutable GridIndex* gridIndex;

    DelaunayTriangulation(const vector<Point>& points);
	DelaunayTriangulation(const DelaunayTriangulation&);
	DelaunayTriangulation& operator = (const DelaunayTriangulation&);

public:
    // creates a Delaunay Triangulation from all the points. duplicated points are ignored.
	static unique_ptr<DelaunayTriangulation> Create(const vector<Point>& points);

	~DelaunayTriangulation();

    // insert the point to this Delaunay Triangulation. Note: if p already exist in this triangulation p is ignored.
    void insertPoint(const Point& p);

    // deletes the given point from this Delaunay Triangulation
    void deletePoint(const Point& p);

    // the number of (distinct) vertices in this triangulation.
    int getSize() const;

    // returns the set of all (distinct) points compusing this triangulation
    const set<Point>& getVertices();

    // computes the current set of all triangles
    vector<Triangle*> getTriangles() const;

    // computes the current set of all triangles
    set<Segment> getSegments() const;

    /**
     * finds the triangle the query point falls in, note if out-side of this
     * triangulation a half plane triangle will be returned;
     * the search has expected time of O(n^0.5) and it starts form a fixed triangle (startTriangle)
     */
    Triangle* find(const Point& p) const;

    // returns a point from the trangulation that is closest to the given point
    Point findClosestPoint(const Point& p) const;

    // Index the triangulation using a grid index
    void InitializeIndex(int xCellCount, int yCellCount) const;
	void InitializeIndex() const;

private:
	// INSERTION
    Triangle* insertPointSimple(const Point& p);

    void startTriangulation(const Point& p1, const Point& p2);

    void insertCollinear(const Point& p, PointSegmentRelation res);

    Triangle* extendInside(Triangle* t, const Point& p);

    Triangle* treatDegeneracyInside(const Triangle* t, const Point& p);

    Triangle* extendOutside(Triangle* t, const Point& p);

    Triangle* extendCounterClockwise(Triangle* t, const Point& p);

    Triangle* extendClockwise(Triangle* t, const Point& p);

    void flipTriangle(Triangle* t, set<Triangle*>& updatedTriangles);

	// DELETION
    // determines if a given point lies on the boundary of the triangulation
    bool isOnBoundary(const Point& p) const;

    //updates the trangulation after the triangles to be deleted and the triangles to be added were found
    void deleteUpdate(const Point& p, vector<Triangle*>&, set<Triangle*>&);

    //update the neighbors of the addedTriangle and deletedTriangle, we assume the 2 triangles share a segment
    void updateNeighbor(Triangle* addedTriangle, Triangle* deletedTriangle, const Point& p);

    //update the neighbors of the 2 added triangles; we assume the 2 triangles share a segment
    void updateNeighbor(Triangle* addedTriangle1, Triangle* addedTriangle2);

    // find triangle to be added to the triangulation after deleting point
    Triangle* findTriangle(vector<Point>& points, const Point& p);

    //finds the a point on the triangle that if connect it to "point" (creating a segment)
    //the other two points of the triangle will be to the left and to the right of the segment
    bool findDiagonal(const Triangle* triangle, const Point& p, Point& res);

    // returns all the points of the triangles that shares point as a corner (the method also saves the triangles that were found)
    vector<Point> findConnectedVertices(const Point& p, vector<Triangle*>& deletedTriangles) const;

    // walks on a consistent side of triangles until a cycle is achieved
    vector<Triangle*> findTriangleNeighborhood(const Point& p, Triangle* start) const;


	// SEARCHING
    /**
     * finds the triangle the query point falls in, note if out-side of this
     * triangulation a half plane triangle will be returned (see contains). the
     * search starts from the the start triangle
     */
    Triangle* find(const Point& p, Triangle* start) const;

    // returns the next triangle for find
    Triangle* findNextTriangle(const Point& p, const Triangle* triangle) const;


	// MISC
    // calculates a Voronoi cell for a given neighborhood (defined by a triangle and one of its corners)
    vector<Point> calcVoronoiCell(const Point& p, Triangle* triangle);

    void updateBoundingBox(const Point& p);
};

} // namespace geometry
