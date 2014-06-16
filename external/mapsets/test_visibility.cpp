#include "test_visibility.h"
#include "dot_parser.h"
#include "graph_algorithms.h"

#include "vis_2approx_points.h"
//#include "vis_shortest_tree_points.h"
#include "vis_shortest_edge_points.h"

vector<Point> GenerateRandomPoints(int n, const Point& center)
{
	vector<Point> res;
	for (int i = 0; i < n; i++)
	{
		double x = randDouble(0, 1000.0);
		double y = randDouble(0, 1000.0);

		res.push_back(Point(center.x + x, center.y + y));
	}

	return res;
}

void TestVisibilityAlgorithms()
{
	VD alg1, alg2, alg3, alg4, alg5;

	for (int i = 0; i < 1; i++)
	{
		srand(i);

		printf("test %d:\n", i);

		int n = randInt(30, 50);
		int m = randInt(30, 50);

		vector<Point> red = GenerateRandomPoints(n, Point(0, 0));
		vector<Point> blue = GenerateRandomPoints(m, Point(0, 0));
		vector<Point> green = GenerateRandomPoints(m, Point(0, 0));

		DotParser parser;
		Graph g = parser.parseGraph("data/sample.gv");
		//Graph g = parser.parseGraph("data//sample4.gv");
		//assert(g.GetClusters().size() == 2);
		red = g.GetClusterPositions("1");
		blue = g.GetClusterPositions("2");
		green = g.GetClusterPositions("3");

		map<string, vector<Point> > trees;
		trees["red"] = red;
		trees["blue"] = blue;
		trees["green"] = green;

		double mstRed = LengthMinimumSpanningTree(red);
		double mstBlue = LengthMinimumSpanningTree(blue);
		double mstGreen = LengthMinimumSpanningTree(green);
		double lb = 1;//mstRed + mstBlue + mstGreen;

		//1
		double alg_1 = min(3*mstRed + mstBlue, mstRed + 3*mstBlue);
		alg1.push_back(alg_1/lb);

		//2
		vector<Point> rb = red;
		rb.insert(rb.end(), blue.begin(), blue.end());
		double alg_2 = 0;//3*MinimumSpanningTree(rb);
		alg2.push_back(alg_2/lb);

		//3
		Vis2ApproxPoints vis2Approx;
		map<string, SegmentTree*> res3 = vis2Approx.BuildTrees(trees);
		OutputDebugTrees(trees, res3, "res3.svg");
		vis2Approx.CheckTrees(res3, trees);

		double alg_3 = vis2Approx.TreeLength(res3);
		alg3.push_back(alg_3/lb);

		//4
		//VisIterativeTree visIterativeTree;
		double alg_4 = 0;//visIterativeTree.TreeLength(visIterativeTree.BuildTrees(trees));
		alg4.push_back(alg_4/lb);
		//assert(alg_4 >= lb);

		//5
		//VisIterativeEdge visIterativeEdge;
		//map<string, SegmentTree*> res5 = visIterativeEdge.BuildTrees(trees);
		//visIterativeEdge.CheckTrees(res5, trees);
		//OutputDebugTrees(trees, res5, "res5.svg");

		double alg_5 = 0;//visIterativeEdge.TreeLength(res5);
		alg5.push_back(alg_5/lb);
		//assert(alg_5 >= lb - EPS);

		printf("ALG_1:  %.3lf\n", alg_1/lb);
		printf("ALG_2:  %.3lf\n", alg_2/lb);
		printf("ALG_3:  %.3lf\n", alg_3/lb);
		//printf("ALG_4:  %.3lf\n", alg_4/lb);
		//printf("ALG_5:  %.3lf\n", alg_5/lb);

		printf("\n");

		
	}

	printf("Worst ALG_1: %.3lf\n", MaximumValue(alg1));
	printf("Worst ALG_2: %.3lf\n", MaximumValue(alg2));
	printf("Worst ALG_3: %.5lf\n", MaximumValue(alg3));
	//printf("Worst ALG_4: %.5lf\n", MaximumValue(alg4));
	//printf("Worst ALG_5: %.5lf\n", MaximumValue(alg5));

	OutputTimeInfo("all done");
}

