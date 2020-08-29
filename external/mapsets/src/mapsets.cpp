#include "mapsets.h"

#include "common/common.h"
#include "common/random_utils.h"
#include "common/graph/dot_parser.h"
#include "common/geometry/segment.h"

#include "graph_algorithms.h"
#include "fd_adjustment.h"
#include "closest_point.h"
#include "visibility.h"
#include "visibility_utils.h"
#include "cest_2approx.h"

namespace mapsets {

void Postprocessing(DotGraph& g)
{
	//fixing node shapes
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		if (!g.nodes[i]->hasAttr("shape"))
			g.nodes[i]->setAttr("shape", "plaintext");
	}
}

void BuildSpanningSubgraphs(DotGraph& g, map<string, SegmentSet*>& trees)
{
	//make it in some other order
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentSet* tree = (*iter).second;
		vector<Point> points = tree->getPoints();

		vector<Segment> boundaryObstacles = GetBoundaryObstacles(g, clusterId, 1.3);
		vector<Segment> treeObstacles = GetTreeObstacles(trees, clusterId);

		//tree.clear();
		for (int i = 0; i < (int)points.size(); i++)
			for (int j = i+1; j < (int)points.size(); j++)
			{
				Segment s = Segment(points[i], points[j]);
				if (tree->contains(s)) continue;

				if (Segment::EdgesIntersect(s, boundaryObstacles) || Segment::EdgesIntersect(s, treeObstacles)) continue;

				tree->append(s);
			}
	}
}

void AddDummyVerticesAlongTreeEdges(DotGraph& g, const map<string, SegmentSet*>& trees)
{
	int DUMMY_VERTICES_CNT = 25;

	ClosestPointQP cp(g.GetBoundingBox());
	map<Point, string> point2Cluster;
	map<string, string> cluster2ClusterColor;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		if (g.nodes[i]->IsDummy()) continue;

		Point p = g.nodes[i]->getPos();
		string clusterId = g.nodes[i]->getCluster();
		string clusterColor = g.nodes[i]->getClusterColor();
		cp.addPoint(p);
		point2Cluster[p] = clusterId;
		cluster2ClusterColor[clusterId] = clusterColor;

		vector<Segment> boundary = g.nodes[i]->getBoundary(1.0);
		for (int j = 0; j < (int)boundary.size(); j++)
		{
			Segment s = boundary[j];
			cp.addPoint(s.first);
			point2Cluster[s.first] = clusterId;

			cp.addPoint(s.second);
			point2Cluster[s.second] = clusterId;

			cp.addPoint(s.middle());
			point2Cluster[s.middle()] = clusterId;
		}
	}

	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		string clusterColor = cluster2ClusterColor[clusterId];
		SegmentSet* tree = (*iter).second;

		for (int i = 0; i < tree->count(); i++)
		{
			Segment seg = tree->get(i);
			Point delta = seg.second - seg.first;
			int cnt = min(DUMMY_VERTICES_CNT, int(seg.length()/15.0));
			cnt = max(cnt, 2);

			delta /= (double)(cnt + 1);

			for (int k = 0; k <= cnt; k++)
			{
				//construct intermediate point
				Point p = seg.first + delta * double(k + 1);

				//adding the point
				//cp.addPoint(p);
				//point2Cluster[p] = clusterId;

				double F = 1.5;
				p += Point::RandomPoint(-F, F, -F, F);
				g.AddDummyPoint(p, clusterId, clusterColor);
			}
		}
	}

	return;
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentSet* tree = (*iter).second;

		for (int i = 0; i < tree->count(); i++)
		{
			Segment seg = tree->get(i);
			Point delta = seg.second - seg.first;
			int cnt = min(DUMMY_VERTICES_CNT, int(seg.length()/15.0));
			cnt = max(cnt, 2);

			delta /= (double)(cnt + 1);

			for (int k = 0; k < cnt; k++)
			{
				//construct intermediate point
				Point p = seg.first + delta * double(k + 1);

				//do we need the point?
				vector<Point> clp = cp.getKClosest(p, 5);
				bool needPoint = false;
				for (int r = 0; r < (int)clp.size(); r++)
				{
					assert(point2Cluster.find(clp[r]) != point2Cluster.end());
					string closestCluster = point2Cluster[clp[r]];
					if (closestCluster != clusterId) 
					{
						needPoint = true;
						break;
					}
				}

				if (!needPoint) continue;

				//adding the point
				double F = 1.5;
				p += Point::RandomPoint(-F, F, -F, F);
				g.AddDummyPoint(p, clusterId, "");

				cp.addPoint(p);
				point2Cluster[p] = clusterId;
			}
		}
	}
}

void AddDummyVerticesAlongLabels(DotGraph& g)
{
	int DUMMY_VERTICES_CNT = 15;

	int nc = (int)g.nodes.size();
	for (int i = 0; i < nc; i++)
	{
		double w = g.nodes[i]->getWidth();
		double h = g.nodes[i]->getHeight();

		double per = 0;
		vector<Segment> boundary = g.nodes[i]->getBoundary(1.0);
		for (int j = 0; j < (int)boundary.size(); j++)
			per += boundary[j].length();

		//double step = per/DUMMY_VERTICES_CNT;
		for (int j = 0; j < (int)boundary.size(); j++)
		{
			int R = (int)(boundary[j].length()*DUMMY_VERTICES_CNT/per);
			for (int k = 0; k < R; k++)
			{
				Point dir = boundary[j].second - boundary[j].first;
				dir *= double(k)/double(R);

				Point p = boundary[j].first + dir;
				double rc = 0.03;
				Point rnd = Point::RandomPoint(-rc*w, rc*w, -rc*h, rc*h);
				p += rnd;

				g.AddDummyPoint(p, g.nodes[i]->getCluster(), g.nodes[i]->getClusterColor());
			}
		}

	}
}

void BuildTrees(DotGraph& g)
{
	// find spanning trees for each cluster
	CEST2Approx vis2Approx;
	auto trees = vis2Approx.BuildTrees(g);

	// pulling tree segments away from obstacles
	ForceDirectedAdjustment(g, trees);

	// adding as many non-intersecting inter-cluster edges as possible
	//BuildSpanningSubgraphs(g, trees);

	// adding dummy vertices for labels
	AddDummyVerticesAlongLabels(g);

	// adding dummy vertices for tree edges
	AddDummyVerticesAlongTreeEdges(g, trees);

	Postprocessing(g);
}

} // namespace mapsets

