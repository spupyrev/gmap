#pragma once

#include "common.h"

struct CMDArgs
{
	CMDArgs() {}

	string inputFile;
	string outputFile;

	bool setParam(const string& name, const string& value)
	{
		if (name == "-o") outputFile = value;
		else 
			return false;

		return true;
	}
};

void usage(int argc, char **argv)
{
	cerr << "Usage: \n";
	cerr << "  "  << argv[0] << " -o=<outputfile> <inputfile>\n";
	exit(-1);
}

void ParseCommandLine(int argc, char **argv, CMDArgs& res)
{
	map<string, string> options;
	for (int i = 1; i < argc; i++)
	{
		vector<string> tmp = splitNotNull(string(argv[i]), "=");
		if (tmp.size() == 1) 
		{
			res.inputFile = tmp[0];
		}
		else if (tmp.size() == 2)
		{
			options[tmp[0]] = tmp[1];
		}
		else 
		{
			usage(argc, argv);
			exit(1);
		}
	}

	for (map<string, string>::iterator it = options.begin(); it != options.end(); it++)
	{
		string name = (*it).first;
		string value = (*it).second;

		if (!res.setParam(name, value)) 
		{
			usage(argc, argv);
			exit(1);
		}
	}
}
