#include "common/common.h"
#include "common/random_utils.h"
#include "common/cmd_options.h"

#include "mapsets.h"

void PrepareCMDOptions(int argc, char** argv, CMDOptions& args)
{
	string msg;
	msg += "Usage: mapsets [options] input_file\n";
	msg += "When input_file is not supplied, the program reads from stdin.\n";
	args.SetUsageMessage(msg);

	args.AddAllowedOption("", "", "Input file name (stdin, if no input file is supplied)");
	args.AddAllowedOption("-o", "", "Output file name (stdout, if no output file is supplied)");

	args.Parse(argc, argv);
}

DotGraph ReadGraph(const string& filename)
{
	DotReader parser;
	DotGraph g = parser.ReadGraph(filename);

	//g.OutputStatistics();

	return g;
}

void WriteGraph(const string& filename, DotGraph& g)
{
	//g.OutputStatistics();

	DotWriter writer;
	writer.WriteGraph(filename, g);
}

int main(int argc, char **argv)
{
	InitRand(123);

	auto options = unique_ptr<CMDOptions>(new CMDOptions());

	int returnCode = 0;
	try
	{
		PrepareCMDOptions(argc, argv, *options);

		DotGraph graph = ReadGraph((*options).getOption(""));
		mapsets::BuildTrees(graph);
		WriteGraph((*options).getOption("-o"), graph);
	}
	catch (int code)
	{
		returnCode = code;		
	}

	return returnCode;
}
