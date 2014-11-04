#include "common.h"
#include "graph.h"
#include "dot_parser.h"
#include "clustering.h"
#include "metrics.h"
#include "segment.h"
#include "cmdargs.h"
#include "random_utils.h"


void doMetricsAction(CMDArgs& args)
{
	DotParser parser;
	Graph g = parser.parseGraph(args.inputFile);
		
	Metrics m = computeMetrics(g);
	m.output();
}

void doClusteringAction(CMDArgs& args)
{
	DotParser parser;
	Graph g = parser.parseGraph(args.inputFile);

	ClusterAlgorithm* algo;
	if (args.C == "geometrickmeans")
	{
		algo = new GeometricKMeans();
	}
	else if (args.C == "graphkmeans")
	{
		algo = new GraphKMeans();
	}
	else if (args.C == "geometrichierarchical")
	{
		algo = new GeometricHierarchical();
	}
	else if (args.C == "graphhierarchical")
	{
		algo = new GraphHierarchical();
	}
	else if (args.C == "infomap")
	{
		algo = new InfoMap();
	}
	else
	{
		cerr<<"Unknown clustering type: '"<<args.C<<"'\n";
		exit(-1);
	}

	if (args.K == "")
	{
		algo->cluster(g);
	}
	else if (args.K == "graph")
	{
		int k = g.numberOfClusters();
		algo->cluster(g, k);
	}
	else
	{
		int k = string2Int(args.K);
		algo->cluster(g, k);
	}
	delete algo;

	DotWriter writer;
	writer.writeGraph(args.outputFile, g);
}

int main(int argc, char **argv)
{
	InitRand(123);

	CMDArgs args;
	ParseCommandLine(argc, argv, args);

	if (args.action == "clustering")
	{
		doClusteringAction(args);
	}
	else if (args.action == "metrics")
	{
		doMetricsAction(args);
	}
	else
	{
		cerr<<"Unknown command-line argument 'action': '"<<args.action<<"'\n";
		usage(argc, argv);
	}

	return 0;
}
