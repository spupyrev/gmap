#pragma once

#include "delaunay_triangulation.h"

namespace geometry {

struct Cell
{
	int x, y;

	Cell(int x = 0, int y = 0): x(x), y(y) {}
};

class DelaunayTriangulation;

/**
 * Grid Index is a simple spatial index for fast point/triangle location.
 * The idea is to divide a predefined geographic extent into equal sized
 * cell matrix (tiles). Every cell will be associated with a triangle which lies inside.
 * Therfore, one can easily locate a triangle in close proximity of the required
 * point by searching from the point's cell triangle. If the triangulation is
 * more or less uniform and bound in space, this index is very effective,
 * roughly recuing the searched triangles by square(xCellCount * yCellCount),
 * as only the triangles inside the cell are searched.
 *
 * The index takes xCellCount * yCellCount capacity. While more cells allow
 * faster searches, even a small grid is helpfull.
 *
 * This implementation holds the cells in a memory matrix, but such a grid can
 * be easily mapped to a DB table or file where it is usually used for it's fullest.
 *
 * Note that the index is geographically bound - only the region given in the
 * c'tor is indexed. Added Triangles outside the indexed region will cause rebuilding of
 * the whole index. Since triangulation is mostly always used for static raster data,
 * and usually is never updated outside the initial zone (only refininf existing triangles)
 * this is never an issue in real life.
 */
class GridIndex
{
private:
    // The triangulation of the index
    const DelaunayTriangulation* indexDelaunay;

    // Horizontal geographic size of a cell
    double x_size;

    // Vertical geographic size of a cell
    double y_size;

    // The indexed geographic size
    Rectangle indexRegion;

    // A division of indexRegion to a cell matrix, where each cell holds a triangle which lies in it
    vector<vector<Triangle*> > grid;

public:
    /**
     * Constructs a grid index holding the triangles of a delaunay triangulation.
     * This version uses the bounding box of the triangulation as the region to index.
     */
    GridIndex(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount);

    /**
     * Constructs a grid index holding the triangles of a delaunay triangulation.
     * The grid will be made of (xCellCount * yCellCount) cells.
     * The smaller the cells the less triangles that fall in them, whuch means better
     * indexing, but also more cells in the index, which mean more storage.
     * The smaller the indexed region is, the smaller the cells can be and still
     * maintain the same capacity, but adding geometries outside the initial region
     * will invalidate the index !
     */
    GridIndex(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount, const Rectangle& region);

	~GridIndex();

    /**
     * Finds a triangle near the given point
     */
    Triangle* findCellTriangle(const Point& point);

    /**
    * Updates the grid index to reflect changes to the triangulation. Note that added
    * triangles outside the indexed region will force to recompute the whole index
    * with the enlarged region.
    */
    void updateIndex(const set<Triangle*>& updatedTriangles);

private:
	GridIndex(const GridIndex&);
	GridIndex& operator = (const GridIndex&);

    // Initialize the grid index
    void init(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount, const Rectangle& region);

    void updateCellValues(int startXCell, int startYCell, int lastXCell, int lastYCell, Triangle* startTriangle);

    // Finds a valid (existing) trinagle adjacent to a given invalid cell
    Triangle* findValidTriangle(const Cell& minInvalidCell) const;

    // Locates the grid cell point covering the given coordinate
    Cell getCell(const Point& p) const;

    // Create a point at the center of a cell
    Point middleOfCell(int x_index, int y_index) const;
};

} // namespace geometry
