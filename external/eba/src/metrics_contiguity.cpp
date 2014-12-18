#include "metrics.h"

#include "common/geometry/delaunay_triangulation.h"

using namespace geometry;

double computeContiguity(DotGraph& g)
{
	vector<double> error;
	for (auto cl : g.GetClusters())
	{
		string cluster = cl.first;

		set<Point> points;
		for (auto node : cl.second)
		{
			Rectangle rect = node->getBoundingRectangle();
			points.insert(Point(rect.xl, rect.yl));
			points.insert(Point(rect.xl, rect.yr));
			points.insert(Point(rect.xr, rect.yl));
			points.insert(Point(rect.xr, rect.yr));
		}
		
		if ((int)points.size() <= 2) continue;

		auto dt = DelaunayTriangulation::Create(vector<Point>(points.begin(), points.end()));
		dt->InitializeIndex();

		// count misclassified nodes
		int correct = 0;
		int incorrect = 0;
		for (auto node : g.nodes)
			if (node->getCluster() != cluster)
			{
				Point pos = node->getPos();
				Triangle* triangle = dt->find(pos);
				if (triangle->halfplane)
				{
					//outside
					correct++;
				}
				else
				{
					//inside
					incorrect++;
				}
			}

		double er = double(incorrect) / double(correct + incorrect);
		assert(0.0 <= er && er <= 1.0);
		error.push_back(er);

		//cerr << cluster << ": " << incorrect << " / " << correct << endl;
	}

	return 1.0 - Average(error);
}

