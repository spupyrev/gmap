#pragma once

#define PLOGP(p) (p > 0.0 ? (p*log(p)) : 0.0)

#include <fstream>

#include "Greedy.h" 
#include "IMNode.h" 

#include "../common.h"
#include "../graph.h"

using namespace std;

struct TreeNode
{
	int level;
	double codeLength;
	set<int> members;
	//  vector<int> cluster; // Two-level partition
	vector<int> rev_renumber; // Vectors to reorganize node numbers
	map<int,int> renumber;
	multimap<double, TreeNode, greater<double> > nextLevel;
};

struct PrintTreeNode
{
	int rank;
	double size;
	multimap<double,int,greater<double> > members;
};

struct TreeStats
{
	double twoLevelCodeLength;
	int Nmodules;
	int largeModuleLimit;
	int NlargeModules;
	double aveDepth;
	double aveSize;
};

vector<vector<Node*> > runInfomap(ConnectedGraph& g, int trials, double rec);
