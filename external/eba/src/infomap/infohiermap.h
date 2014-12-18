#pragma once

#include "greedy.h"
#include "imnode.h"

#include "common/graph/dot_graph.h"

#include <functional>

namespace infomap {

#define PLOGP(p) (p > 0.0 ? (p*log(p)) : 0.0)

struct TreeNode
{
	int level;
	double codeLength;
	set<int> members;
	//  vector<int> cluster; // Two-level partition
	vector<int> rev_renumber; // Vectors to reorganize node numbers
	map<int, int> renumber;
	multimap<double, TreeNode, greater<double> > nextLevel;
};

struct PrintTreeNode
{
	int rank;
	double size;
	multimap<double, int, greater<double> > members;
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

vector<vector<DotNode*> > runInfomap(ConnectedDotGraph& g, int trials, double rec);

} // namespace infomap
