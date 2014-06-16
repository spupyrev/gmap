#include "common.h"
#include "random_utils.h"
#include "time_utils.h"
#include "cmdargs.h"
#include "dot_parser.h"
#include "segment.h"
#include "graph_algorithms.h"
#include "fd_adjustment.h"
#include "closest_point.h"
#include "debug_utils.h"
#include "visibility.h"
#include "visibility_utils.h"
#include "test_visibility.h"
#include "vis_2approx.h"

Graph ReadGraph(const string& filename)
{
	DotParser parser;
	Graph g = parser.parseGraph(filename);
	OutputTimeInfo("#nodes = %d  #edges = %d  #clusters = %d", g.nodes.size(), g.edges.size(), g.GetClusterCount());

	return g;
}

void WriteGraph(const string& filename, Graph& g)
{
	OutputTimeInfo("#nodes = %d  #edges = %d  #clusters = %d", g.nodes.size(), g.edges.size(), g.GetClusterCount());

	//fixing node shapes
	for (int i = 0; i < g.nodes.size(); i++)
	{
		if (!g.nodes[i]->hasAttr("shape"))
			g.nodes[i]->setAttr("shape", "plaintext");
	}

	DotWriter writer;
	writer.writeGraph(filename, g);
}

void BuildSpanningSubgraphs(Graph& g, map<string, SegmentTree*>& trees)
{
	//make it in some other order
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentTree* tree = (*iter).second;
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

void AddDummyVerticesAlongTreeEdges(Graph& g, const map<string, SegmentTree*>& trees)
{
	int DUMMY_VERTICES_CNT = 25;

	ClosestPointQP cp(g.GetBoundingBox());
	map<Point, string> point2Cluster;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		if (g.nodes[i]->IsDummy()) continue;

		Point p = g.nodes[i]->getPos();
		string clusterId = g.nodes[i]->getCluster();
		cp.addPoint(p);
		point2Cluster[p] = clusterId;

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
		SegmentTree* tree = (*iter).second;

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
				g.AddDummyPoint(p, clusterId);
			}
		}
	}

	return;
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		string clusterId = (*iter).first;
		SegmentTree* tree = (*iter).second;

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
				g.AddDummyPoint(p, clusterId);

				cp.addPoint(p);
				point2Cluster[p] = clusterId;
			}
		}
	}
}

void AddDummyVerticesAlongLabels(Graph& g)
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

		double step = per/DUMMY_VERTICES_CNT;
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

				g.AddDummyPoint(p, g.nodes[i]->getCluster());
			}
		}

	}
}

void BuildTrees(const CMDArgs& args)
{
	Graph g = ReadGraph(args.inputFile);

	//find spanning trees for each cluster
	Vis2Approx vis2Approx;
	//double ink = 1;//vis2Approx.GetMSTLength(g);
	//OutputTimeInfo("MST-Ink: %.3lf", ink);

	//OutputTimeInfo("Ink-2-Approx: %.3lf", vis2Approx.Get2ApproxLength(g)/ink);

	map<string, SegmentTree*> trees = vis2Approx.BuildTrees(g);
	OutputTimeInfo("spanning trees computed");
	//OutputTimeInfo("Ink: %.3lf", vis2Approx.TreeLength(trees)/ink);
	//OutputDebugTrees(g, trees, "trees.svg");

	//pulling tree segments away from obstacles
	ForceDirectedAdjustment(g, trees);
	OutputTimeInfo("force-directed adjustments finished");
	//OutputDebugTrees(g, trees, "trees-fd.svg");
	//OutputTimeInfo("Ink-FD: %.3lf", vis2Approx.TreeLength(trees)/ink);

	//adding as many non-intersecting inter-cluster edges as possible
	//BuildSpanningSubgraphs(g, trees);
	OutputTimeInfo("spanning subgraphs computed");
	//OutputDebugTrees(g, trees, "subgraphs.svg");

	//adding dummy vertices for labels
	AddDummyVerticesAlongLabels(g);
	//OutputTimeInfo("vertices for labels added");

	//adding dummy vertices for tree edges
	AddDummyVerticesAlongTreeEdges(g, trees);
	//OutputTimeInfo("vertices along edges added");

	WriteGraph(args.outputFile, g);
}

int main(int argc, char **argv)
{
	InitRand(123);
	//InitTime("log");

	//TestClosestPoints();
	//TestSegmentIntersections();
	//TestVisibilityAlgorithms();
	//return 0;

	CMDArgs args;
	//args.inputFile = "data/map.dot";
	//args.outputFile = "sample-res.gv";

	ParseCommandLine(argc, argv, args);

	BuildTrees(args);

	//FinalizeTime();
	return 0;
}
