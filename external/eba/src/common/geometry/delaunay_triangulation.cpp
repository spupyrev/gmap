#include "delaunay_triangulation.h"

#include <iostream>
#include <algorithm>
#include <queue>
#include <cassert>

namespace geometry {

DelaunayTriangulation::DelaunayTriangulation(const vector<Point>& points)
{
    allPointsCollinear = true;
	firstT = lastT = nullptr;
	startTriangle = nullptr;
	boundingBox = nullptr;
	gridIndex = nullptr;

    for (const Point& p : points)
    {
        insertPoint(p);
    }
}

unique_ptr<DelaunayTriangulation> DelaunayTriangulation::Create(const vector<Point>& points)
{
	return unique_ptr<DelaunayTriangulation>(new DelaunayTriangulation(points));
}

DelaunayTriangulation::~DelaunayTriangulation()
{
	for (auto t : getTriangles())
		delete t;

	vertices.clear();
	delete boundingBox;
	delete gridIndex;
}

int DelaunayTriangulation::getSize() const
{
    return (int)vertices.size();
}

const set<Point>& DelaunayTriangulation::getVertices()
{
    return vertices;
}

vector<Triangle*> DelaunayTriangulation::getTriangles() const
{
	vector<Triangle*> triangles;

    if ((int)vertices.size() > 2)
    {
        queue<Triangle*> q;
		set<Triangle*> used;

		q.push(startTriangle);
		used.insert(startTriangle);

        while (!q.empty())
        {
			Triangle* t = q.front(); q.pop();

            triangles.push_back(t);
            if (t->abnext != nullptr && !used.count(t->abnext))
            {
                q.push(t->abnext);
				used.insert(t->abnext);
            }
            if (t->bcnext != nullptr && !used.count(t->bcnext))
            {
                q.push(t->bcnext);
				used.insert(t->bcnext);
            }
            if (t->canext != nullptr && !used.count(t->canext))
            {
                q.push(t->canext);
				used.insert(t->canext);
            }
        }
    }

    return triangles;
}

set<Segment> DelaunayTriangulation::getSegments() const
{
	set<Segment> result;
	if ((int)vertices.size() == 2)
	{
		result.insert(Segment(firstP, lastP));
	}

	for (auto t : getTriangles())
	{
		result.insert(Segment(t->a, t->b));
		if (!t->halfplane)
		{
			result.insert(Segment(t->a, t->c));
			result.insert(Segment(t->b, t->c));
		}
	}

	return result;
}

void DelaunayTriangulation::InitializeIndex(int xCellCount, int yCellCount)	const
{
	if (startTriangle != nullptr)
		gridIndex = new GridIndex(this, xCellCount, yCellCount);
}

void DelaunayTriangulation::InitializeIndex() const
{
	int cellCount = (int)sqrt(double(vertices.size())) + 1;
	InitializeIndex(cellCount, cellCount);
}

/*
 * INSERTION
*/
void DelaunayTriangulation::insertPoint(const Point& p)
{
    if (vertices.count(p))
        return;

    updateBoundingBox(p);
    vertices.insert(p);
    Triangle* t = insertPointSimple(p);
    if (t == nullptr)
        return;

    // The triangles that are affected in the current operation
    set<Triangle*> updatedTriangles;

    Triangle* tt = t;
    do
    {
        flipTriangle(tt, updatedTriangles);
        tt = tt->canext;
    }
    while (tt != t && !tt->halfplane);

    // Update index with changed triangles
	if (gridIndex != nullptr)
        gridIndex->updateIndex(updatedTriangles);
}

Triangle* DelaunayTriangulation::insertPointSimple(const Point& p)
{
    if (!allPointsCollinear)
    {
        Triangle* t = find(p, startTriangle);
        if (t->halfplane)
            startTriangle = extendOutside(t, p);
        else
            startTriangle = extendInside(t, p);

        return startTriangle;
    }

	if ((int)vertices.size() == 1)
    {
        firstP = p;
        return nullptr;
    }

    if ((int)vertices.size() == 2)
    {
        startTriangulation(firstP, p);
        return nullptr;
    }

    switch (Triangle::PointLineTest(p, firstP, lastP))
    {
    case LEFT:
        startTriangle = extendOutside(firstT->abnext, p);
        allPointsCollinear = false;
        break;
    case RIGHT:
        startTriangle = extendOutside(firstT, p);
        allPointsCollinear = false;
        break;
    case ONSEGMENT:
        insertCollinear(p, ONSEGMENT);
        break;
    case INFRONTOFA:
        insertCollinear(p, INFRONTOFA);
        break;
    case BEHINDB:
        insertCollinear(p, BEHINDB);
        break;
    }

    return nullptr;
}

void DelaunayTriangulation::startTriangulation(const Point& p1, const Point& p2)
{
    Point ps, pb;
    if (p1 < p2)
    {
        ps = p1;
        pb = p2;
    }
    else
    {
        ps = p2;
        pb = p1;
    }

    firstT = new Triangle(pb, ps);
    lastT = firstT;
    Triangle* t = new Triangle(ps, pb);
    firstT->abnext = t;
    t->abnext = firstT;
    firstT->bcnext = t;
    t->canext = firstT;
    firstT->canext = t;
    t->bcnext = firstT;

    firstP = firstT->b;
    lastP = lastT->a;
}

void DelaunayTriangulation::insertCollinear(const Point& p, PointSegmentRelation res)
{
    Triangle* t;
	Triangle* tp;
	Triangle* u;

    switch (res)
    {
    case INFRONTOFA:
        t = new Triangle(firstP, p);
        tp = new Triangle(p, firstP);
        t->abnext = tp;
        tp->abnext = t;
        t->bcnext = tp;
        tp->canext = t;
        t->canext = firstT;
        firstT->bcnext = t;
        tp->bcnext = firstT->abnext;
        firstT->abnext->canext = tp;
        firstT = t;
        firstP = p;
        break;
    case BEHINDB:
        t = new Triangle(p, lastP);
        tp = new Triangle(lastP, p);
        t->abnext = tp;
        tp->abnext = t;
        t->bcnext = lastT;
        lastT->canext = t;
        t->canext = tp;
        tp->bcnext = t;
        tp->canext = lastT->abnext;
        lastT->abnext->bcnext = tp;
        lastT = t;
        lastP = p;
        break;
    case ONSEGMENT:
        u = firstT;
        while (p > u->a)
            u = u->canext;

        t = new Triangle(p, u->b);
        tp = new Triangle(u->b, p);
        u->b = p;
        u->abnext->a = p;
        t->abnext = tp;
        tp->abnext = t;
        t->bcnext = u->bcnext;
        u->bcnext->canext = t;
        t->canext = u;
        u->bcnext = t;
        tp->canext = u->abnext->canext;
        u->abnext->canext->bcnext = tp;
        tp->bcnext = u->abnext;
        u->abnext->canext = tp;
        if (firstT == u)
        {
            firstT = t;
        }
        break;
    default:
		assert(false);
    }
}

Triangle* DelaunayTriangulation::extendInside(Triangle* t, const Point& p)
{
    Triangle* h1 = treatDegeneracyInside(t, p);
    if (h1 != nullptr)
        return h1;

    h1 = new Triangle(t->c, t->a, p);
    Triangle* h2 = new Triangle(t->b, t->c, p);
    t->c = p;
    t->InitCircumcircle();
    h1->abnext = t->canext;												
    h1->bcnext = t;
    h1->canext = h2;
    h2->abnext = t->bcnext;
    h2->bcnext = h1;
    h2->canext = t;
	h1->abnext->SwitchNeighbors(t, h1);
	h2->abnext->SwitchNeighbors(t, h2);
    t->bcnext = h2;
    t->canext = h1;
    return t;
}

Triangle* DelaunayTriangulation::treatDegeneracyInside(const Triangle* t, const Point& p)
{
    if (t->abnext->halfplane && Triangle::PointLineTest(p, t->b, t->a) == ONSEGMENT)
        return extendOutside(t->abnext, p);
    if (t->bcnext->halfplane && Triangle::PointLineTest(p, t->c, t->b) == ONSEGMENT)
        return extendOutside(t->bcnext, p);
    if (t->canext->halfplane && Triangle::PointLineTest(p, t->a, t->c) == ONSEGMENT)
        return extendOutside(t->canext, p);

    return nullptr;
}

Triangle* DelaunayTriangulation::extendOutside(Triangle* t, const Point& p)
{
    if (Triangle::PointLineTest(p, t->a, t->b) == ONSEGMENT)
    {
        Triangle* dg = new Triangle(t->a, t->b, p);
        Triangle* hp = new Triangle(p, t->b);
        t->b = p;
        dg->abnext = t->abnext;
		dg->abnext->SwitchNeighbors(t, dg);
        dg->bcnext = hp;
        hp->abnext = dg;
        dg->canext = t;
        t->abnext = dg;
        hp->bcnext = t->bcnext;
        hp->bcnext->canext = hp;
        hp->canext = t;
        t->bcnext = hp;
        return dg;
    }

    Triangle* ccT = extendCounterClockwise(t, p);
    Triangle* cT = extendClockwise(t, p);
    ccT->bcnext = cT;
    cT->canext = ccT;
    return cT->abnext;
}

Triangle* DelaunayTriangulation::extendCounterClockwise(Triangle* t, const Point& p)
{
    t->halfplane = false;
    t->c = p;
    t->InitCircumcircle();

    Triangle* tca = t->canext;

    if (Triangle::PointLineTest(p, tca->a, tca->b) >= RIGHT)
    {
        Triangle* nT = new Triangle(t->a, p);
        nT->abnext = t;
        t->canext = nT;
        nT->canext = tca;
        tca->bcnext = nT;
        return nT;
    }
    return extendCounterClockwise(tca, p);
}

Triangle* DelaunayTriangulation::extendClockwise(Triangle* t, const Point& p)
{
    t->halfplane = false;
    t->c = p;
    t->InitCircumcircle();

    Triangle* tbc = t->bcnext;

    if (Triangle::PointLineTest(p, tbc->a, tbc->b) >= RIGHT)
    {
        Triangle* nT = new Triangle(p, t->b);
        nT->abnext = t;
        t->bcnext = nT;
        nT->bcnext = tbc;
        tbc->canext = nT;
        return nT;
    }

    return extendClockwise(tbc, p);
}

void DelaunayTriangulation::flipTriangle(Triangle* t, set<Triangle*>& updatedTriangles)
{
	updatedTriangles.insert(t);

    Triangle* u = t->abnext;
	if (u->halfplane || !u->circumcircle.Contains(t->c))
        return;

	//cout << u->id << endl;
	Triangle* v;
    if (t->a == u->a)
    {
        v = new Triangle(u->b, t->b, t->c);
        v->abnext = u->bcnext;
        t->abnext = u->abnext;
    }
    else if (t->a == u->b)
    {
        v = new Triangle(u->c, t->b, t->c);
        v->abnext = u->canext;
        t->abnext = u->bcnext;
    }
    else if (t->a == u->c)
    {
        v = new Triangle(u->a, t->b, t->c);
        v->abnext = u->abnext;
        t->abnext = u->canext;
    }

	updatedTriangles.insert(v);
    v->bcnext = t->bcnext;
	v->abnext->SwitchNeighbors(u, v);
	v->bcnext->SwitchNeighbors(t, v);
    t->bcnext = v;
    v->canext = t;
    t->b = v->a;
	t->abnext->SwitchNeighbors(u, t);
    t->InitCircumcircle();

	delete u;

    flipTriangle(t, updatedTriangles);
    flipTriangle(v, updatedTriangles);
}

/*
 * DELETION
*/
void DelaunayTriangulation::deletePoint(const Point& p)
{
	if (!vertices.count(p))
		return;

	if (isOnBoundary(p))
	{
		cerr << "Can't delete point (" << p.x << "," << p.y << ") on the perimeter" << endl;
		return;
	}

    // The triangles that are deleted in the current deletePoint iteration
    vector<Triangle*> deletedTriangles;
    // The triangles that are added in the current deletePoint iteration
    set<Triangle*> addedTriangles;

    // Finding the triangles to delete
    vector<Point> points = findConnectedVertices(p, deletedTriangles);
    while ((int)points.size() >= 3)
    {
        // Getting a triangle to add, and saving it
        Triangle* triangle = findTriangle(points, p);
        addedTriangles.insert(triangle);

        // Finding the point on the diagonal
        Point tmp;
		if (findDiagonal(triangle, p, tmp))
		{
			points.erase(remove(points.begin(), points.end(), tmp), points.end()); 
		}
    }

	if (std::find(deletedTriangles.begin(), deletedTriangles.end(), startTriangle) != deletedTriangles.end())
		startTriangle = *addedTriangles.begin();

	vertices.erase(vertices.find(p));

    //updating the trangulation
    deleteUpdate(p, deletedTriangles, addedTriangles);
	for (auto t : deletedTriangles)
		delete t;
}

bool DelaunayTriangulation::isOnBoundary(const Point& p) const
{
    // Getting one of the neigh
    Triangle* triangle = find(p);
	assert(triangle->isCorner(p));

    Triangle* prevTriangle = nullptr;
    Triangle* currentTriangle = triangle;
	Triangle* nextTriangle = currentTriangle->NextNeighbor(p, prevTriangle);

    while (nextTriangle != triangle)
    {
        //the point is on the perimeter
        if (nextTriangle->halfplane)
            return true;

        prevTriangle = currentTriangle;
        currentTriangle = nextTriangle;
        nextTriangle = currentTriangle->NextNeighbor(p, prevTriangle);
    }

    return false;
}

vector<Point> DelaunayTriangulation::findConnectedVertices(const Point& p, vector<Triangle*>& deletedTriangles)	const
{
    // getting one of the neighbors
    Triangle* triangle = find(p);
	assert(triangle->isCorner(p));

	deletedTriangles = findTriangleNeighborhood(p, triangle);

    vector<Point> result;
    set<Point> pointsSet;
    for (auto t : deletedTriangles)
    {
        if (t->a == p && !pointsSet.count(t->b))
        {
            pointsSet.insert(t->b);
			result.push_back(t->b);
        }

        if (t->b == p && !pointsSet.count(t->c))
        {
            pointsSet.insert(t->c);
			result.push_back(t->c);
        }

        if (t->c == p && !pointsSet.count(t->a))
        {
            pointsSet.insert(t->a);
			result.push_back(t->a);
        }
    }

	return result;
}

vector<Triangle*> DelaunayTriangulation::findTriangleNeighborhood(const Point& p, Triangle* firstTriangle) const
{
	vector<Triangle*> result;
    result.push_back(firstTriangle);

    Triangle* prevTriangle = nullptr;
    Triangle* currentTriangle = firstTriangle;
	Triangle* nextTriangle = currentTriangle->NextNeighbor(p, prevTriangle);

    while (nextTriangle != firstTriangle)
    {
        //the point is NOT on the perimeter
		assert(!nextTriangle->halfplane);

        result.push_back(nextTriangle);
        prevTriangle = currentTriangle;
        currentTriangle = nextTriangle;
        nextTriangle = currentTriangle->NextNeighbor(p, prevTriangle);
    }

    return result;
}

Triangle* DelaunayTriangulation::findTriangle(vector<Point>& points, const Point& p)
{
	int n = (int)points.size();
    if (n < 3)
    {
        return nullptr;
    }
    else if (n == 3)
    {
	    // if we left with 3 points, we return the triangle
        return new Triangle(points[0], points[1], points[2]);
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            Point p1 = points[i];
            Point p2 = points[(i+1)%n];
            Point p3 = points[(i+2)%n];

            //check if the triangle is not re-entrant and not encloses p
            Triangle* t = new Triangle(p1, p2, p3);
            if (Triangle::isConvex(p1, p2, p3) && !t->Contains(p))
            {
                if (!t->InsideCircumcircle(points))
                    return t;
            }

            //if there are only 4 points use contains that refers to point on boundary as outside
            if (n == 4 && Triangle::isConvex(p1, p2, p3) && !(t->Contains(p) && !t->OnBoundary(p)))
            {
                if (!t->InsideCircumcircle(points))
                    return t;
            }

			delete t;
        }
    }

    return nullptr;
}

bool DelaunayTriangulation::findDiagonal(const Triangle* t, const Point& point, Point& res)
{
    if (Triangle::PointLineTest(t->a, point, t->c) == LEFT && Triangle::PointLineTest(t->b, point, t->c) == RIGHT)
	{
		res = t->c;
        return true;
	}
    if (Triangle::PointLineTest(t->c, point, t->b) == LEFT && Triangle::PointLineTest(t->a, point, t->b) == RIGHT)
	{
		res = t->b;
        return true;
	}
    if (Triangle::PointLineTest(t->b, point, t->a) == LEFT && Triangle::PointLineTest(t->c, point, t->a) == RIGHT)
	{
		res = t->a;
        return true;
	}

    return false;
}

void DelaunayTriangulation::deleteUpdate(const Point& p, vector<Triangle*>& deletedTriangles, set<Triangle*>& addedTriangles)
{
    for (auto addedTriangle1 : addedTriangles)
    {
        //update between addedd triangles and deleted triangles
        for (auto deletedTriangle : deletedTriangles)
        {
            if (addedTriangle1->ShareSide(deletedTriangle))
            {
                updateNeighbor(addedTriangle1, deletedTriangle, p);
            }
        }
    }
    for (auto addedTriangle1 : addedTriangles)
    {
        //update between added triangles
        for (auto addedTriangle2 : addedTriangles)
        {
            if (addedTriangle1 != addedTriangle2 && addedTriangle1->ShareSide(addedTriangle2))
            {
                updateNeighbor(addedTriangle1, addedTriangle2);
            }
        }
    }

    // Update index with changed triangles
    if (gridIndex != nullptr)
        gridIndex->updateIndex(addedTriangles);
}

void DelaunayTriangulation::updateNeighbor(Triangle* addedTriangle, Triangle* deletedTriangle, const Point& p)
{
    Point delA = deletedTriangle->a;
    Point delB = deletedTriangle->b;
    Point delC = deletedTriangle->c;
    Point addA = addedTriangle->a;
    Point addB = addedTriangle->b;
    Point addC = addedTriangle->c;

    //updates the neighbor of the deleted triangle to point to the added triangle
    //setting the neighbor of the added triangle
    if (p == delA)
    {
		deletedTriangle->bcnext->SwitchNeighbors(deletedTriangle, addedTriangle);
        //AB-BC || BA-BC
        if ((addA == delB && addB == delC) || (addB == delB && addA == delC))
        {
            addedTriangle->abnext = deletedTriangle->bcnext;
        }
        //AC-BC || CA-BC
        else if ((addA == delB && addC == delC) || (addC == delB && addA == delC))
        {
            addedTriangle->canext = deletedTriangle->bcnext;
        }
        //BC-BC || CB-BC
        else
        {
            addedTriangle->bcnext = deletedTriangle->bcnext;
        }
    }
    else if (p == delB)
    {
		deletedTriangle->canext->SwitchNeighbors(deletedTriangle, addedTriangle);
        //AB-AC || BA-AC
        if ((addA == delA && addB == delC) || (addB == delA && addA == delC))
        {
            addedTriangle->abnext = deletedTriangle->canext;
        }
        //AC-AC || CA-AC
        else if ((addA == delA && addC == delC) || (addC == delA && addA == delC))
        {
            addedTriangle->canext = deletedTriangle->canext;
        }
        //BC-AC || CB-AC
        else
        {
            addedTriangle->bcnext = deletedTriangle->canext;
        }
    }
    //equals c
    else
    {
		deletedTriangle->abnext->SwitchNeighbors(deletedTriangle, addedTriangle);
        //AB-AB || BA-AB
        if ((addA == delA && addB == delB) || (addB == delA && addA == delB))
        {
            addedTriangle->abnext = deletedTriangle->abnext;
        }
        //AC-AB || CA-AB
        else if ((addA == delA && addC == delB) || (addC == delA && addA == delB))
        {
            addedTriangle->canext = deletedTriangle->abnext;
        }
        //BC-AB || CB-AB
        else
        {
            addedTriangle->bcnext = deletedTriangle->abnext;
        }
    }
}

void DelaunayTriangulation::updateNeighbor(Triangle* addedTriangle1, Triangle* addedTriangle2)
{
    Point A1 = addedTriangle1->a;
    Point B1 = addedTriangle1->b;
    Point C1 = addedTriangle1->c;
    Point A2 = addedTriangle2->a;
    Point B2 = addedTriangle2->b;
    Point C2 = addedTriangle2->c;

    //A1-A2
    if (A1 == A2)
    {
        //A1B1-A2B2
        if (B1 == B2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //A1B1-A2C2
        else if (B1 == C2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //A1C1-A2B2
        else if (C1 == B2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //A1C1-A2C2
        else
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
    }
    //A1-B2
    else if (A1 == B2)
    {
        //A1B1-B2A2
        if (B1 == A2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //A1B1-B2C2
        else if (B1 == C2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //A1C1-B2A2
        else if (C1 == A2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //A1C1-B2C2
        else
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
    //A1-C2
    else if (A1 == C2)
    {
        //A1B1-C2A2
        if (B1 == A2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //A1B1-C2B2
        if (B1 == B2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //A1C1-C2A2
        if (C1 == A2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //A1C1-C2B2
        else
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
    //B1-A2
    else if (B1 == A2)
    {
        //B1A1-A2B2
        if (A1 == B2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //B1A1-A2C2
        else if (A1 == C2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //B1C1-A2B2
        else if (C1 == B2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //B1C1-A2C2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
    }
    //B1-B2
    else if (B1 == B2)
    {
        //B1A1-B2A2
        if (A1 == A2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //B1A1-B2C2
        else if (A1 == C2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //B1C1-B2A2
        else if (C1 == A2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //B1C1-B2C2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
    //B1-C2
    else if (B1 == C2)
    {
        //B1A1-C2A2
        if (A1 == A2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //B1A1-C2B2
        if (A1 == B2)
        {
            addedTriangle1->abnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //B1C1-C2A2
        if (C1 == A2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //B1C1-C2B2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
    //C1-A2
    else if (C1 == A2)
    {
        //C1A1-A2B2
        if (A1 == B2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //C1A1-A2C2
        else if (A1 == C2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //C1B1-A2B2
        else if (B1 == B2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //C1B1-A2C2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
    }
    //C1-B2
    else if (C1 == B2)
    {
        //C1A1-B2A2
        if (A1 == A2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //C1A1-B2C2
        else if (A1 == C2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //C1B1-B2A2
        else if (B1 == A2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->abnext = addedTriangle1;
        }
        //C1B1-B2C2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
    //C1-C2
    else if (C1 == C2)
    {
        //C1A1-C2A2
        if (A1 == A2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //C1A1-C2B2
        if (A1 == B2)
        {
            addedTriangle1->canext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
        //C1B1-C2A2
        if (B1 == A2)
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->canext = addedTriangle1;
        }
        //C1B1-C2B2
        else
        {
            addedTriangle1->bcnext = addedTriangle2;
            addedTriangle2->bcnext = addedTriangle1;
        }
    }
}


/*
 * SEARCH
*/
Triangle* DelaunayTriangulation::find(const Point& p) const
{
    // If triangulation has a spatial index try to use it as the starting triangle
    Triangle* searchTriangle = startTriangle;
    if (gridIndex != nullptr)
    {
        Triangle* indexTriangle = gridIndex->findCellTriangle(p);
        if (indexTriangle != nullptr)
            searchTriangle = indexTriangle;
    }

    // Search for the point's triangle starting from searchTriangle
    return find(p, searchTriangle);
}

Triangle* DelaunayTriangulation::find(const Point& p, Triangle* start) const
{
    Triangle* next_t;
    if (start->halfplane)
    {
        next_t = findNextTriangle(p, start);
        if (next_t == nullptr || next_t->halfplane)
            return start;
        start = next_t;
    }

    while (true)
    {
        next_t = findNextTriangle(p, start);
        if (next_t == nullptr)
            return start;
        if (next_t->halfplane)
            return next_t;
        start = next_t;
    }
}

Triangle* DelaunayTriangulation::findNextTriangle(const Point& p, const Triangle* triangle)	const
{
	if (!triangle->halfplane)
	{
		if (Triangle::PointLineTest(p, triangle->a, triangle->b) == RIGHT && !triangle->abnext->halfplane)
			return triangle->abnext;
		if (Triangle::PointLineTest(p, triangle->b, triangle->c) == RIGHT && !triangle->bcnext->halfplane)
			return triangle->bcnext;
		if (Triangle::PointLineTest(p, triangle->c, triangle->a) == RIGHT && !triangle->canext->halfplane)
			return triangle->canext;
		if (Triangle::PointLineTest(p, triangle->a, triangle->b) == RIGHT)
			return triangle->abnext;
		if (Triangle::PointLineTest(p, triangle->b, triangle->c) == RIGHT)
			return triangle->bcnext;
		if (Triangle::PointLineTest(p, triangle->c, triangle->a) == RIGHT)
			return triangle->canext;
	}
	else
	{
		if (triangle->abnext != nullptr && !triangle->abnext->halfplane)
			return triangle->abnext;
		if (triangle->bcnext != nullptr && !triangle->bcnext->halfplane)
			return triangle->bcnext;
		if (triangle->canext != nullptr && !triangle->canext->halfplane)
			return triangle->canext;
	}

	return nullptr;
}

Point DelaunayTriangulation::findClosestPoint(const Point& p) const
{
    Triangle* triangle = find(p);
    Point p1 = triangle->a;
    Point p2 = triangle->b;
    double d1 = p1.Distance(p);
    double d2 = p2.Distance(p);

    if (triangle->halfplane)
    {
        if (d1 <= d2)
        {
            return p1;
        }
        else
        {
            return p2;
        }
    }
    else
    {
        Point p3 = triangle->c;

        double d3 = p3.Distance(p);
        if (d1 <= d2 && d1 <= d3)
        {
            return p1;
        }
        else if (d2 <= d1 && d2 <= d3)
        {
            return p2;
        }
        else
        {
            return p3;
        }
    }
}

vector<Point> DelaunayTriangulation::calcVoronoiCell(const Point& p, Triangle* triangle)
{
    // handle any full triangle		 
    if (!triangle->halfplane)
    {
        // get all neighbors of given corner point
        vector<Triangle*> neighbors = findTriangleNeighborhood(p, triangle);

        vector<Point> vertices;
        // for each neighbor, including the given triangle, add center of circumscribed circle to cell polygon
		for (auto t : neighbors)
        {
            vertices.push_back(t->circumcircle.getCenter());
        }

        return vertices;
    }
    // handle half plane
    // in this case, the cell is a single line
    // which is the perpendicular bisector of the half plane line
    else
    {
        // local friendly alias			
        Triangle* halfplane = triangle;
        // third point of triangle adjacent to this half plane
        // (the point not shared with the half plane)
        Point third;
        // triangle adjacent to the half plane
        Triangle* neighbor = nullptr;

        // find the neighbor triangle
        if (!halfplane->abnext->halfplane)
        {
            neighbor = halfplane->abnext;
        }
        else if (!halfplane->bcnext->halfplane)
        {
            neighbor = halfplane->bcnext;
        }
        else if (!halfplane->bcnext->halfplane)
        {
            neighbor = halfplane->canext;
        }

        // find third point of neighbor triangle
        // (the one which is not shared with current half plane)
        // this is used in determining half plane orientation
        if (neighbor->a != halfplane->a && neighbor->a != halfplane->b)
            third = neighbor->a;
        if (neighbor->b != halfplane->a && neighbor->b != halfplane->b)
            third = neighbor->b;
        if (neighbor->c != halfplane->a && neighbor->c != halfplane->b)
            third = neighbor->c;

        // delta (slope) of half plane edge
        double halfplane_delta = (halfplane->a.y - halfplane->b.y) / (halfplane->a.x - halfplane->b.x);

        // delta of line perpendicular to current half plane edge
        double perp_delta = (1.0 / halfplane_delta) * (-1.0);

        // determine orientation: find if the third point of the triangle
        // lies above or below the half plane
        // works by finding the matching y value on the half plane line equation
        // for the same x value as the third point
        double y_orient = halfplane_delta * (third.x - halfplane->a.x) + halfplane->a.y;
        bool above = true;
        if (y_orient > third.y)
            above = false;

        // based on orientation, determine cell line direction
        // (towards right or left side of window)
        double sign = 1.0;
        if ((perp_delta < 0 && !above) || (perp_delta > 0 && above))
            sign = -1.0;

        // the cell line is a line originating from the circumcircle to infinity
        // x = 500.0 is used as a large enough value
        Point circumcircle = neighbor->circumcircle.getCenter();
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
		//TODO::::::::::::::::::::::::::::::::::::::::::::::::::::
        double x_cell_line = (circumcircle.x + (500.0 * sign));
        double y_cell_line = perp_delta * (x_cell_line - circumcircle.x) + circumcircle.y;

		vector<Point> result;
		result.push_back(circumcircle);
		result.push_back(Point(x_cell_line, y_cell_line));

        return result;
    }
}

void DelaunayTriangulation::updateBoundingBox(const Point& p)
{
    if (boundingBox == nullptr)
		boundingBox = new Rectangle(p);
    else
		boundingBox->Add(p);
}

} // namespace geometry
