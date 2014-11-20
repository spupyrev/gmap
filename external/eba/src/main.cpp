#include "common/common.h"
#include "common/random_utils.h"
#include "common/cmd_options.h"

#include "common/graph/dot_graph.h"
#include "common/graph/dot_parser.h"

#include "clustering.h"
#include "metrics.h"

void MetricsAction(const CMDOptions* args)
{
	DotReader parser;
	DotGraph g = parser.ReadGraph(args->getOption("-i"));

	Metrics m;
	m.Compute(g);
	m.Output(args->getOption("-o"));
}

void ClusteringAction(const CMDOptions* args)
{
	DotReader parser;
	DotGraph g = parser.ReadGraph(args->getOption("-i"));

	string algoName = args->getOption("-C");
	ClusterAlgorithm* algo = NULL;

	if (algoName == "geometrickmeans")
		algo = new GeometricKMeans();
	else if (algoName == "graphkmeans")
		algo = new GraphKMeans();
	else if (algoName == "geometrichierarchical")
		algo = new GeometricHierarchical();
	else if (algoName == "graphhierarchical")
		algo = new GraphHierarchical();
	else if (algoName == "infomap")
		algo = new InfoMap();
	else
		args->InvalidOption("-C");

	string numberOfClusters = args->getOption("-K");
	if (numberOfClusters == "graph")
	{
		int k = g.ClusterCount();
		algo->cluster(g, k);
	}
	else if (numberOfClusters == "")
	{
		algo->cluster(g);
	}
	else
	{
		int k = toInt(numberOfClusters);
		algo->cluster(g, k);
	}

	delete algo;

	DotWriter writer;
	writer.WriteGraph(args->getOption("-o"), g);
}

CMDOptions* PrepareCMDOptions(int argc, char **argv)
{
	CMDOptions* args = new CMDOptions();
	args->AddAllowedOption("-action", "  -action=value  : action [clustering|metrics]");
	args->AddAllowedOption("-i", "",  "  -i=value       : input file (stdin, if no input file is supplied)");
	args->AddAllowedOption("-o", "",  "  -o=value       : output file (stdout, if no output file is supplied)");
	args->AddAllowedOption("-C",	  "  -C=value       : type of clustering [geometrickmeans|graphkmeans|geometrichierarchical|graphhierarchical|infomap]");
	args->AddAllowedOption("-K", "",  "  -K=value       : desired number of clusters (selected automatically, if no value is supplied)");

	args->Parse(argc, argv);
	return args;
}

int main(int argc, char **argv)
{
	InitRand(123);

	CMDOptions* args = PrepareCMDOptions(argc, argv);

	string action = args->getOption("-action");
	if (action == "clustering")
	{
		ClusteringAction(args);
	}
	else if (action == "metrics")
	{
		MetricsAction(args);
	}
	else
	{
		args->InvalidOption("-action");
	}

	delete args;

	return 0;
}
