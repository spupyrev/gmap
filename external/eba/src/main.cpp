#include "common/common.h"
#include "common/random_utils.h"
#include "common/cmd_options.h"

#include "common/graph/dot_graph.h"
#include "common/graph/dot_parser.h"

#include "clustering.h"
#include "metrics.h"

#include <memory>

void PrepareCMDOptions(int argc, char** argv, CMDOptions& args)
{
	string msg;
	msg += "Usage: kmeans [options] input_file\n";
	msg += "When input_file is not supplied, the program reads from stdin.\n";
	args.SetUsageMessage(msg);

	args.AddAllowedOption("", "", "Input file name (stdin, if no input file is supplied)");
	args.AddAllowedOption("-o", "", "Output file name (stdout, if no output file is supplied)");

	args.AddAllowedOption("-action", "The program either computes a clustering for a given graph or calculates a set of quality measures (aesthetics)");
	args.AddAllowedValue("-action", "clustering");
	args.AddAllowedValue("-action", "metrics");

	args.AddAllowedOption("-C", "Type of clustering");
	args.AddAllowedValue("-C", "geometrickmeans");
	args.AddAllowedValue("-C", "graphkmeans");
	args.AddAllowedValue("-C", "geometrichierarchical");
	args.AddAllowedValue("-C", "graphhierarchical");
	args.AddAllowedValue("-C", "infomap");
	args.AddAllowedValue("-C", "modularity");
	args.AddAllowedValue("-C", "modularity-cont");

	args.AddAllowedOption("-K", "", "Desired number of clusters (selected automatically, if no value is supplied)");

	args.Parse(argc, argv);
} 

void MetricsAction(const CMDOptions& options)
{
	DotReader parser;
	DotGraph g = parser.ReadGraph(options.getOption(""));

	Metrics m;
	m.Compute(g);
	m.Output(options.getOption("-o"));
}

void ClusteringAction(const CMDOptions& options)
{
	DotReader parser;
	DotGraph g = parser.ReadGraph(options.getOption(""));

	string algoName = options.getOption("-C");
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
	else if (algoName == "modularity")
		algo = new Modularity(false);
	else if (algoName == "modularity-cont")
		algo = new Modularity(true);

	string numberOfClusters = options.getOption("-K");
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
	writer.WriteGraph(options.getOption("-o"), g);
}

int main(int argc, char **argv)
{
	InitRand(123);

	auto options = CMDOptions::Create();

	int returnCode = 0;
	try
	{
		PrepareCMDOptions(argc, argv, *options);
		string action = options->getOption("-action");
		if (action == "clustering")
		{
			ClusteringAction(*options);
		}
		else if (action == "metrics")
		{
			MetricsAction(*options);
		}
	}
	catch (int code)
	{
		returnCode = code;		
	}

	return returnCode;
}
