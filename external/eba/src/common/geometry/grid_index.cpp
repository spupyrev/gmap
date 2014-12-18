#include "grid_index.h"

namespace geometry {

/**
    * Constructs a grid index holding the triangles of a delaunay triangulation.
    * This version uses the bounding box of the triangulation as the region to index.
    */
GridIndex::GridIndex(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount)
{
    init(delaunay, xCellCount, yCellCount, *delaunay->boundingBox);
}

/**
    * Constructs a grid index holding the triangles of a delaunay triangulation.
    * The grid will be made of (xCellCount * yCellCount) cells.
    * The smaller the cells the less triangles that fall in them, whuch means better
    * indexing, but also more cells in the index, which mean more storage.
    * The smaller the indexed region is, the smaller the cells can be and still
    * maintain the same capacity, but adding geometries outside the initial region
    * will invalidate the index !
    */
GridIndex::GridIndex(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount, const Rectangle& region)
{
    init(delaunay, xCellCount, yCellCount, region);
}

GridIndex::~GridIndex()
{
	grid.clear();
}

/**
    * Finds a triangle near the given point
*/
Triangle* GridIndex::findCellTriangle(const Point& point)
{
    int x_index = (int)((point.x - indexRegion.minX()) / x_size);
    int y_index = (int)((point.y - indexRegion.minY()) / y_size);

	x_index = max(x_index, 0);
	x_index = min(x_index, (int)grid.size() - 1);
	y_index = max(y_index, 0);
	y_index = min(y_index, (int)grid[0].size() - 1);

    return grid[x_index][y_index];
}

/**
* Updates the grid index to reflect changes to the triangulation. Note that added
* triangles outside the indexed region will force to recompute the whole index
* with the enlarged region.
*/
void GridIndex::updateIndex(const set<Triangle*>& updatedTriangles)
{
	if (updatedTriangles.empty()) return;

    // Gather the bounding box of the updated area
	Rectangle updatedRegion((*updatedTriangles.begin())->a);
	for (Triangle* t : updatedTriangles)
	{
		updatedRegion.Add(t->a);
		updatedRegion.Add(t->b);
		updatedRegion.Add(t->c);
	}

    // Bad news - the updated region lies outside the indexed region.
    // The whole index must be recalculated
    if (!indexRegion.Contains(updatedRegion))
    {
		indexRegion.Add(updatedRegion);
        init(indexDelaunay, (int)(indexRegion.getWidth() / x_size), (int)(indexRegion.getHeight() / y_size), indexRegion);
    }
    else
    {
        // Find the cell region to be updated
        Cell minInvalidCell = getCell(updatedRegion.minPoint());
        Cell maxInvalidCell = getCell(updatedRegion.maxPoint());

        // And update it with fresh triangles
        Triangle* adjacentValidTriangle = findValidTriangle(minInvalidCell);
        updateCellValues(minInvalidCell.x, minInvalidCell.y, maxInvalidCell.x, maxInvalidCell.y, adjacentValidTriangle);
    }
}

// Initialize the grid index
void GridIndex::init(const DelaunayTriangulation* delaunay, int xCellCount, int yCellCount, const Rectangle& region)
{
    indexDelaunay = delaunay;
    indexRegion = region;
    x_size = region.getWidth() / yCellCount;
    y_size = region.getHeight() / xCellCount;

    // The grid will hold a trinagle for each cell, so a point (x,y) will lie
    // in the cell representing the grid partition of region to a
    //  xCellCount on yCellCount grid
    grid = vector<vector<Triangle*> >(xCellCount, vector<Triangle*>(yCellCount));

	Triangle* colStartTriangle = indexDelaunay->find(middleOfCell(0, 0), indexDelaunay->startTriangle);
    updateCellValues(0, 0, xCellCount - 1, yCellCount - 1, colStartTriangle);
}

void GridIndex::updateCellValues(int startXCell, int startYCell, int lastXCell, int lastYCell, Triangle* startTriangle)
{
    // Go over each grid cell and locate a triangle in it to be the cell's
    // starting search triangle. Since we only pass between adjacent cells
    // we can search from the last triangle found and not from the start.

    // Add triangles for each column cells
    for (int i = startXCell; i <= lastXCell; i++)
    {
        // Find a triangle at the begining of the current column
        startTriangle = indexDelaunay->find(middleOfCell(i, startYCell), startTriangle);
        grid[i][startYCell] = startTriangle;
        Triangle* prevRowTriangle = startTriangle;

        // Add triangles for the next row cells
        for (int j = startYCell + 1; j <= lastYCell; j++)
        {
            grid[i][j] = indexDelaunay->find(middleOfCell(i, j), prevRowTriangle);
            prevRowTriangle = grid[i][j];
        }
    }
}

// Finds a valid (existing) trinagle adjacent to a given invalid cell
Triangle* GridIndex::findValidTriangle(const Cell& minInvalidCell) const
{
    // If the invalid cell is the minimal one in the grid we are forced to search the
    // triangulation for a triangle at that location
    if (minInvalidCell.x == 0 && minInvalidCell.y == 0)
	{
		return indexDelaunay->find(middleOfCell(0, 0), indexDelaunay->startTriangle);
	}
    else
	{
        // Otherwise we can take an adjacent cell triangle that is still valid
        return grid[min(minInvalidCell.x, 0)][min(minInvalidCell.y, 0)];
	}
}

// Locates the grid cell point covering the given coordinate
Cell GridIndex::getCell(const Point& p)	const
{
    int xCell = (int)((p.x - indexRegion.minX()) / x_size);
    int yCell = (int)((p.y - indexRegion.minY()) / y_size);
	xCell = min(xCell, (int)grid.size() - 1);
	yCell = min(yCell, (int)grid[0].size() - 1);
    return Cell(xCell, yCell);
}

// Create a point at the center of a cell
Point GridIndex::middleOfCell(int x_index, int y_index) const
{
    double middleXCell = indexRegion.minX() + x_index * x_size + x_size / 2;
    double middleYCell = indexRegion.minY() + y_index * y_size + y_size / 2;
    return Point(middleXCell, middleYCell);
}

} // namespace geometry
