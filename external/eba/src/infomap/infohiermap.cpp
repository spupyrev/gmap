#include "infohiermap.h"

#include "common/random_utils.h"
#include "common/graph/dot_graph.h"

namespace infomap {

#define IM_DEBUG 0

struct InfomapResult
{
	map<string, int> idToIndex;
	map<string, DotNode*> idToNode;

	map<DotNode*, string> code;

	vector<vector<DotNode*> > extractResult(ConnectedDotGraph& g)
	{
		map<string, vector<DotNode*> > cl;

		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			string s = code[g.nodes[i]];
			VS tmp = SplitNotNull(s, ":");
			//taking lowest cluster
			/*string ncl = "";
			for (int j = 0; j+1 < (int)tmp.size(); j++)
				ncl += tmp[j] + ":";
			cl[ncl].push_back(g.nodes[i]);*/
			//taking highest cluster
			cl[tmp[0]].push_back(g.nodes[i]);
		}

		vector<vector<DotNode*> > res;

		for (auto iter = cl.begin(); iter != cl.end(); iter++)
			res.push_back((*iter).second);

		return res;
	}

	vector<vector<DotNode*> > extractDisconnectedResult(ConnectedDotGraph& g)
	{
		vector<vector<DotNode*> > res;

		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			vector<DotNode*> v;
			v.push_back(g.nodes[i]);
			res.push_back(v);
		}

		return res;
	}
};

double fast_hierarchical_partition(IMNode **node, TreeNode &map, double totalDegree, int Nnode, double &twoLevelCodeLength, bool deep);
double repeated_hierarchical_partition(InfomapResult& ir, vector<double> &degree, vector<string> &nodeNames, IMNode **orig_node, TreeNode &map, double totalDegree, int Nnode, int Ntrials, double recursive, TreeStats &stats);
void partition(IMNode ***node, Greedy* greedy);
void repeated_partition(IMNode ***node, Greedy* greedy, int Ntrials);

void printTree(InfomapResult& ir, string s, TreeNode &map, vector<string> &nodeNames, vector<double> &degree, double totalDegree, int depth, TreeStats &stats);
void genSubNet(IMNode **orig_node, int Nnode, IMNode **sub_node, int sub_Nnode, TreeNode &map, double totalDegree);
void setCodeLength(IMNode **orig_node, IMNode **sub_node, int mod, TreeNode &map, double totalDegree);
void addNodesToMap(TreeNode &map, vector<double> &size);
void collapseTree(multimap<double, PrintTreeNode, greater<double> > &collapsedmap, TreeNode &map, vector<double> &size, int level);

vector<vector<DotNode*> > runInfomap(ConnectedDotGraph& g, int trials, double rec)
{
	int Ntrials = trials;  // Set number of partition attempts
	double recursive = rec;
	string line;
	string buf;
	int Nnode = (int)g.nodes.size();
	vector<string> nodeNames(Nnode);
	InfomapResult ir;

	// Read node names
	for (int i = 0; i < Nnode; i++)
	{
		nodeNames[i] = g.nodes[i]->id;
		ir.idToIndex[g.nodes[i]->id] = i;
		ir.idToNode[g.nodes[i]->id] = g.nodes[i];
	}

	double totalDegree = 0.0;
	vector<double> degree(Nnode, 0.0);
	IMNode**node = new IMNode*[Nnode];

	for (int i = 0; i < Nnode; i++)
		node[i] = new IMNode(i);

	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		DotNode* v = g.nodes[i];
		vector<pair<DotNode*, DotEdge*> > adj = g.getAdj(v);

		for (int j = 0; j < (int)adj.size(); j++)
		{
			DotNode* u = adj[j].first;

			if (v->index >= u->index) continue;

			int from = ir.idToIndex[v->id];
			int to = ir.idToIndex[u->id];
			double weight = adj[j].second->getWeight();
			node[from]->links.push_back(make_pair(to, weight));
			node[to]->links.push_back(make_pair(from, weight));
			node[from]->degree += weight;
			node[to]->degree += weight;
			degree[from] += weight;
			degree[to] += weight;
			totalDegree += 2.0 * weight;
		}
	}

	if (totalDegree < 1e-6)
	{
		return ir.extractDisconnectedResult(g);
	}

	// Calculate uncompressed code length
	double uncompressedCodeLength = 0.0;

	for (int i = 0; i < Nnode; i++)
	{
		double p = degree[i] / totalDegree;

		if (p > 0.0)
			uncompressedCodeLength -= p * log(p) / log(2.0);
	}

	// Partition network hierarchically
	TreeNode map;
	TreeStats stats;
	double codeLength = repeated_hierarchical_partition(ir, degree, nodeNames, node, map, totalDegree, Nnode, Ntrials, recursive, stats);

	if (IM_DEBUG)
	{
		cout << endl << "Best codelength = " << codeLength << " bits." << endl;
		cout << "Compression: " << 100.0 * (1.0 - codeLength / uncompressedCodeLength) << " percent." << endl;
		cout << "Number of modules: " << stats.Nmodules << endl;
		cout << "Number of large modules (> 1 percent of total number of nodes): " << stats.NlargeModules << endl;
		cout << "Average depth: " << stats.aveDepth << endl;
		cout << "Average size: " << stats.aveSize << endl;
		cout << "Gain over two-level code: " << 100.0 * (stats.twoLevelCodeLength - codeLength) / codeLength << " percent." << endl;
	}

	for (int i = 0; i < Nnode; i++)
		delete node[i];

	delete[] node;
	return ir.extractResult(g);
}

double fast_hierarchical_partition(IMNode **orig_node, TreeNode &map, double totalDegree, int Nnode, double &twoLevelCodeLength, bool deep)
{
	// Construct sub network
	int sub_Nnode = map.members.size();
	IMNode **sub_node = new IMNode*[sub_Nnode];
	genSubNet(orig_node, Nnode, sub_node, sub_Nnode, map, totalDegree);

	if (sub_Nnode == 1)
	{
		// Clean up
		for (int i = 0; i < sub_Nnode; i++)
			delete sub_node[i];

		delete [] sub_node;
		// Clear deeper hierarchy of previous map
		multimap<double, TreeNode, greater<double> >().swap(map.nextLevel);
		return map.codeLength;
	}

	// Store best map
	double best_codeLength = map.codeLength;
	TreeNode best_map = map;
	// Initiate solver
	Greedy* sub_greedy;
	sub_greedy = new Greedy(sub_Nnode, totalDegree, sub_node);
	sub_greedy->initiate();

	// If a subtree exists, use this information
	if (!map.nextLevel.empty())
	{
		vector<int> cluster = vector<int>(sub_Nnode);
		int sub_Nmod = 0;

		for (multimap<double, TreeNode, greater<double> >::iterator subsub_it = map.nextLevel.begin(); subsub_it != map.nextLevel.end(); subsub_it++)
		{
			set<int> subsub_members = subsub_it->second.members;

			for (set<int>::iterator mem = subsub_members.begin(); mem != subsub_members.end(); mem++)
				cluster[map.renumber[(*mem)]] = sub_Nmod;

			sub_Nmod++;
		}

		sub_greedy->determMove(cluster);
	}

	// Clear deeper hierarchy of previous map
	multimap<double, TreeNode, greater<double> >().swap(map.nextLevel);

	if (IM_DEBUG)
	{
		if (map.level == 1)
			cout << "Partition first level into..." << flush;
	}

	// Partition partition --> index codebook + module codebook
	partition(&sub_node, sub_greedy);

	if (map.level == 1)
	{
		if (IM_DEBUG)
			cout << sub_greedy->Nnode << " module(s) with code length " << sub_greedy->codeLength << " bits" << endl;

		if (sub_greedy->codeLength < twoLevelCodeLength)
			twoLevelCodeLength = sub_greedy->codeLength;
	}

	double subIndexLength = sub_greedy->indexLength;
	double subCodeLength = sub_greedy->codeLength;

	// Continue only if the network splits
	if (sub_greedy->Nnode == 1)
	{
		// Clean up
		for (int i = 0; i < sub_greedy->Nnode; i++)
			delete sub_node[i];

		delete [] sub_node;
		delete sub_greedy;
		return map.codeLength;
	}
	else
	{
		// Store temporary result
		vector<vector<int> > members = vector<vector<int> >(sub_greedy->Nnode);

		for (int i = 0; i < sub_greedy->Nnode; i++)
		{
			int Nmembers = sub_node[i]->members.size();
			members[i] = vector<int>(Nmembers);

			for (int j = 0; j < Nmembers; j++)
			{
				members[i][j] = map.rev_renumber[sub_node[i]->members[j]];
			}
		}

		// Extend map with the new level
		int Nm = members.size();

		for (int i = 0; i < Nm; i++)
		{
			TreeNode sub_map;
			double degree = 0.0;
			int Nn = members[i].size();

			for (int j = 0; j < Nn; j++)
			{
				int member = members[i][j];
				degree += orig_node[member]->degree;
				sub_map.members.insert(member);
			}

			sub_map.level = map.level + 1;
			setCodeLength(orig_node, sub_node, i, sub_map, totalDegree);
			map.nextLevel.insert(make_pair(degree / totalDegree, sub_map));
		}

		map.codeLength = subIndexLength;
		best_codeLength = subCodeLength;
		best_map = map;

		if (!deep)
		{
			// Clean up
			for (int i = 0; i < sub_greedy->Nnode; i++)
				delete sub_node[i];

			delete [] sub_node;
			delete sub_greedy;
			return subCodeLength;
		}

		double codeLength = map.codeLength;
		// Add index codebooks as long as the code gets shorter
		int Nmod = 2 * sub_greedy->Nnode;

		while (sub_greedy->Nnode > 1 && sub_greedy->Nnode != Nmod) // Continue as long as the network can be partitioned and the result is non-trivial
		{
			// Add index codebook <--> move up in hierarchy
			Nmod = sub_greedy->Nnode;

			for (int i = 0; i < sub_greedy->Nnode; i++)
				vector<int>(1, i).swap(sub_node[i]->members);

			sub_greedy->initiate();

			if (IM_DEBUG)
			{
				if (map.level == 1)
					cout << "Trying to add index codebook..." << flush;
			}

			partition(&sub_node, sub_greedy);
			map.codeLength = sub_greedy->moduleLength;
			subIndexLength = sub_greedy->moduleLength;  // Because the module has been collapsed

			// If trivial result
			if (sub_greedy->Nnode > 1 && sub_greedy->Nnode != Nmod)
			{
				// Store temporary result
				vector<vector<int> > newMembers = vector<vector<int> >(sub_greedy->Nnode);

				for (int i = 0; i < sub_greedy->Nnode; i++)
				{
					int Nmembers = sub_node[i]->members.size();

					for (int j = 0; j < Nmembers; j++)
					{
						copy(members[sub_node[i]->members[j]].begin(), members[sub_node[i]->members[j]].end(), back_inserter(newMembers[i]));
					}
				}

				vector<vector<int> >(newMembers).swap(members);
				// Delete subtrees before generating new ones
				multimap<double, TreeNode, greater<double> >().swap(map.nextLevel);
				// Store result as one-level subtrees
				int Nm = members.size();

				for (int i = 0; i < Nm; i++)
				{
					TreeNode sub_map;
					double degree = 0.0;
					int Nn = members[i].size();

					for (int j = 0; j < Nn; j++)
					{
						int member = members[i][j];
						degree += orig_node[member]->degree;
						sub_map.members.insert(member);
					}

					sub_map.level = map.level + 1;
					map.nextLevel.insert(make_pair(degree / totalDegree, sub_map));
				}

				if (IM_DEBUG)
				{
					if (map.level == 1)
						cout << "succeeded. " << sub_greedy->Nnode << " modules with estimated code length... " << flush;
				}

				// Create hierarchical tree under current level recursively
				codeLength = sub_greedy->indexLength;

				for (multimap<double, TreeNode, greater<double> >::iterator it = map.nextLevel.begin(); it != map.nextLevel.end(); it++)
					codeLength += fast_hierarchical_partition(orig_node, it->second, totalDegree, Nnode, twoLevelCodeLength, false);

				if (IM_DEBUG)
				{
					if (map.level == 1)
						cout << codeLength << " bits." << endl;
				}

				if (codeLength < best_codeLength - 1.0e-10)  // Improvement
				{
					map.codeLength = sub_greedy->indexLength;
					subIndexLength = sub_greedy->indexLength;
					best_codeLength = codeLength;
					best_map = map;
				}
				else  // Longer code, restore best result and stop
				{
					if (IM_DEBUG)
					{
						if (map.level == 1)
							cout << "No improvement. Now trying to create deeper structure based on best result." << endl;
					}

					break;
				}
			}
			else
			{
				if (IM_DEBUG)
				{
					if (map.level == 1)
						cout << "failed. Now trying to create deeper structure based on best result." << endl;
				}
			}
		}

		// Clean up
		for (int i = 0; i < sub_greedy->Nnode; i++)
			delete sub_node[i];

		delete [] sub_node;
		delete sub_greedy;
		// Restore best map
		map = best_map;
	}

	// Create hierarchical tree under current level recursively
	double codeLength = map.codeLength;

	for (multimap<double, TreeNode, greater<double> >::iterator it = map.nextLevel.begin(); it != map.nextLevel.end(); it++)
		codeLength += fast_hierarchical_partition(orig_node, it->second, totalDegree, Nnode, twoLevelCodeLength, true);

	// Update best map if improvements
	if (codeLength < best_codeLength - 1.0e-10)
	{
		best_codeLength = codeLength;
	}
	else
	{
		map = best_map;
	}

	return best_codeLength;
}

double repeated_hierarchical_partition(InfomapResult& ir, vector<double> &degree, vector<string> &nodeNames, IMNode **orig_node, TreeNode &best_map, double totalDegree, int Nnode, int Ntrials, double recursive, TreeStats &stats)
{
	double shortestCodeLength = 1000.0;
	stats.twoLevelCodeLength = 1000.0;

	for (int trial = 0; trial < Ntrials; trial++)
	{
		if (IM_DEBUG)
			cout << "Attempt " << trial + 1 << "/" << Ntrials << ":" << endl;

		TreeNode map;
		map.level = 1;

		for (int i = 0; i < Nnode; i++)
			map.members.insert(i);

		double codeLength = fast_hierarchical_partition(orig_node, map, totalDegree, Nnode, stats.twoLevelCodeLength, true);

		if (IM_DEBUG)
			cout << "Code length = " << codeLength << " bits." << endl;

		if (codeLength < shortestCodeLength)
		{
			shortestCodeLength = codeLength;
			best_map = map;
			//Print hierarchical partition
			//ostringstream oss;
			//ofstream outfile;
			//outfile.open(oss.str().c_str());
			//outfile << "# Codelength = " << codeLength << " bits." << endl;
			string s;
			int depth = 1;
			stats.aveDepth = 0.0;
			stats.aveSize = 0.0;
			stats.Nmodules = 0;
			stats.NlargeModules = 0;
			stats.largeModuleLimit = static_cast<int>(0.01 * Nnode);
			printTree(ir, s, map, nodeNames, degree, totalDegree, depth, stats);
			//outfile.close();
			stats.aveDepth /= 1.0 * Nnode;
			stats.aveSize /= 1.0 * Nnode;

			if (IM_DEBUG)
			{
				cout << "done!" << endl;
				cout << "Average depth: " << stats.aveDepth << endl;
				cout << "Average size: " << stats.aveSize << endl;
				cout << "Number of modules: " << stats.Nmodules << endl;
				cout << "Number of large modules (> 1 percent of total number of nodes): " << stats.NlargeModules << endl;
				cout << "Gain over two-level code: " << 100.0 * (stats.twoLevelCodeLength - codeLength) / codeLength << " percent." << endl;
			}
		}
	}

	return shortestCodeLength;
}

void partition(IMNode ***node, Greedy *greedy)
{
	int Nnode = greedy->Nnode;
	IMNode **cpy_node = new IMNode*[Nnode];

	for (int i = 0; i < Nnode; i++)
	{
		cpy_node[i] = new IMNode((*node)[i]);
	}

	int iteration = 0;
	double outer_oldCodeLength;

	do
	{
		outer_oldCodeLength = greedy->codeLength;

		if ((iteration > 0) && (iteration % 2 == 0) && (greedy->Nnode > 1))
		{
			// Partition the partition
			IMNode **rpt_node = new IMNode*[Nnode];

			for (int i = 0; i < Nnode; i++)
			{
				rpt_node[i] = new IMNode(cpy_node[i]);
			}

			vector<int> subMoveTo = vector<int>(Nnode);
			vector<int> moveTo = vector<int>(Nnode);
			int subModIndex = 0;

			for (int i = 0; i < greedy->Nnode; i++)
			{
				int sub_Nnode = (*node)[i]->members.size();

				if (sub_Nnode > 1)
				{
					IMNode **sub_node = new IMNode*[sub_Nnode];
					set<int> sub_mem;

					for (int j = 0; j < sub_Nnode; j++)
						sub_mem.insert((*node)[i]->members[j]);

					set<int>::iterator it_mem = sub_mem.begin();
					int *sub_renumber = new int[Nnode];
					int *sub_rev_renumber = new int[sub_Nnode];
					double totalDegree = 0.0;

					for (int j = 0; j < sub_Nnode; j++)
					{
						int orig_nr = (*it_mem);
						int orig_Nlinks = cpy_node[orig_nr]->links.size(); // ERROR HERE
						sub_renumber[orig_nr] = j;
						sub_rev_renumber[j] = orig_nr;
						sub_node[j] = new IMNode(j);

						for (int k = 0; k < orig_Nlinks; k++)
						{
							int orig_link = cpy_node[orig_nr]->links[k].first;
							int orig_link_newnr = sub_renumber[orig_link];
							double orig_weight = cpy_node[orig_nr]->links[k].second;

							if (orig_link < orig_nr)
							{
								if (sub_mem.find(orig_link) != sub_mem.end())
								{
									sub_node[j]->links.push_back(make_pair(orig_link_newnr, orig_weight));
									sub_node[orig_link_newnr]->links.push_back(make_pair(j, orig_weight));
									totalDegree += 2.0 * orig_weight;
								}
							}
						}

						it_mem++;
					}

					Greedy* sub_greedy;
					sub_greedy = new Greedy(sub_Nnode, totalDegree, sub_node);
					sub_greedy->initiate();
					partition(&sub_node, sub_greedy);

					for (int j = 0; j < sub_greedy->Nnode; j++)
					{
						int Nmembers = sub_node[j]->members.size();

						for (int k = 0; k < Nmembers; k++)
						{
							subMoveTo[sub_rev_renumber[sub_node[j]->members[k]]] = subModIndex;
						}

						moveTo[subModIndex] = i;
						subModIndex++;
						delete sub_node[j];
					}

					delete [] sub_node;
					delete sub_greedy;
					delete [] sub_renumber;
					delete [] sub_rev_renumber;
				}
				else
				{
					subMoveTo[(*node)[i]->members[0]] = subModIndex;
					moveTo[subModIndex] = i;
					subModIndex++;
				}
			}

			for (int i = 0; i < greedy->Nnode; i++)
				delete (*node)[i];

			delete [] (*node);
			greedy->Nnode = Nnode;
			greedy->Nmod = Nnode;
			greedy->node = rpt_node;
			greedy->initiate();
			greedy->determMove(subMoveTo);
			greedy->level(node, false);
			greedy->determMove(moveTo);
			(*node) = rpt_node;
			outer_oldCodeLength = greedy->codeLength;
		}
		else if (iteration > 0)
		{
			IMNode **rpt_node = new IMNode*[Nnode];

			for (int i = 0; i < Nnode; i++)
			{
				rpt_node[i] = new IMNode(cpy_node[i]);
			}

			vector<int> moveTo = vector<int>(Nnode);

			for (int i = 0; i < greedy->Nnode; i++)
			{
				int Nmembers = (*node)[i]->members.size();

				for (int j = 0; j < Nmembers; j++)
				{
					moveTo[(*node)[i]->members[j]] = i;
				}
			}

			for (int i = 0; i < greedy->Nnode; i++)
				delete (*node)[i];

			delete [] (*node);
			greedy->Nnode = Nnode;
			greedy->Nmod = Nnode;
			greedy->node = rpt_node;
			greedy->initiate();
			greedy->determMove(moveTo);
			(*node) = rpt_node;
		}
		else
		{
		}

		double oldCodeLength;

		do
		{
			oldCodeLength = greedy->codeLength;
			bool moved = true;
			int Nloops = 0;
			int count = 0;

			while (moved)
			{
				moved = false;
				double inner_oldCodeLength = greedy->codeLength;
				greedy->move(moved);
				Nloops++;
				count++;

				if (fabs(inner_oldCodeLength - greedy->codeLength) < 1.0e-10)
					moved = false;

				if (count == 10)
				{
					greedy->tune();
					count = 0;
				}
			}

			greedy->level(node, true);
		}
		while (oldCodeLength - greedy->codeLength >  1.0e-10);

		iteration++;
	}
	while (outer_oldCodeLength - greedy->codeLength > 1.0e-10);

	for (int i = 0; i < Nnode; i++)
		delete cpy_node[i];

	delete [] cpy_node;
}

void repeated_partition(IMNode ***node, Greedy *greedy, int Ntrials)
{
	double shortestCodeLength = 1000.0;
	int Nnode = greedy->Nnode;
	vector<int> cluster = vector<int>(Nnode);

	for (int trial = 0; trial < Ntrials; trial++)
	{
		IMNode **cpy_node = new IMNode*[Nnode];

		for (int i = 0; i < Nnode; i++)
		{
			cpy_node[i] = new IMNode((*node)[i]);
		}

		greedy->Nnode = Nnode;
		greedy->Nmod = Nnode;
		greedy->node = cpy_node;
		greedy->initiate();
		partition(&cpy_node, greedy);

		if (greedy->codeLength < shortestCodeLength)
		{
			shortestCodeLength = greedy->codeLength;

			// Store best partition
			for (int i = 0; i < greedy->Nnode; i++)
			{
				for (vector<int>::iterator mem = cpy_node[i]->members.begin(); mem != cpy_node[i]->members.end(); mem++)
				{
					cluster[(*mem)] = i;
				}
			}
		}

		for (int i = 0; i < greedy->Nnode; i++)
		{
			delete cpy_node[i];
		}

		delete [] cpy_node;
	}

	// Commit best partition
	greedy->Nnode = Nnode;
	greedy->Nmod = Nnode;
	greedy->node = (*node);
	greedy->initiate();
	greedy->determMove(cluster);
	greedy->level(node, true);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void genSubNet(IMNode **orig_node, int Nnode, IMNode **sub_node, int sub_Nnode, TreeNode &map, double totalDegree)
{
	vector<int>(sub_Nnode).swap(map.rev_renumber);
	//vector<int>(Nnode,-1).swap(map.renumber);
	std::map<int, int>().swap(map.renumber);
	vector<double> degree = vector<double>(sub_Nnode, 0.0);
	double exit = 0.0;
	double flow = 0.0;
	// Construct sub network
	set<int>::iterator it_mem = map.members.begin();

	for (int i = 0; i < sub_Nnode; i++)
	{
		int orig_nr = (*it_mem);
		map.renumber[orig_nr] = i;
		map.rev_renumber[i] = orig_nr;
		it_mem++;
	}

	it_mem = map.members.begin();

	for (int i = 0; i < sub_Nnode; i++)
	{
		int orig_nr = (*it_mem);
		int orig_Nlinks = orig_node[orig_nr]->links.size();
		sub_node[i] = new IMNode(i);

		for (int j = 0; j < orig_Nlinks; j++)
		{
			int orig_link = orig_node[orig_nr]->links[j].first;
			int orig_link_newnr = -1;
			std::map<int, int>::iterator it = map.renumber.find(orig_link);

			if (it != map.renumber.end())
				orig_link_newnr = it->second;

			//int orig_link_newnr = map.renumber[orig_link];
			double orig_weight = orig_node[orig_nr]->links[j].second;
			degree[i] += orig_weight;
			flow += orig_weight;

			if (orig_link_newnr < 0)
			{
				sub_node[i]->outDegree += orig_weight;
				exit += orig_weight;
				flow += orig_weight;
			}
			else if (orig_link < orig_nr) // Should be <= if self-links are included
			{
				if (map.members.find(orig_link) != map.members.end())
				{
					sub_node[i]->links.push_back(make_pair(orig_link_newnr, orig_weight));
					sub_node[orig_link_newnr]->links.push_back(make_pair(i, orig_weight));
				}
			}
		}

		it_mem++;
	}

	double codeLength = 0.0;

	for (int i = 0; i < sub_Nnode; i++)
		codeLength -= PLOGP(degree[i] / flow);

	codeLength -= PLOGP(exit / flow);
	codeLength *= flow / totalDegree / log(2.0);
	map.codeLength = codeLength;
}

void printTree(InfomapResult& ir, string s, TreeNode &map, vector<string>& nodeNames, vector<double> &degree, double totalDegree, int depth, TreeStats &stats)
{
	if (map.nextLevel.size() > 0)
	{
		int i = 1;

		for (multimap<double, TreeNode, greater<double> >::iterator it = map.nextLevel.begin(); it != map.nextLevel.end(); it++)
		{
			stats.Nmodules++;

			if (1.0 * it->second.members.size() > 1.0 * stats.largeModuleLimit)
				stats.NlargeModules++;

			string cpy_s(s + to_string(i) + ":");
			printTree(ir, cpy_s, it->second, nodeNames, degree, totalDegree, depth + 1, stats);
			i++;
		}
	}
	else
	{
		stats.aveDepth += 1.0 * map.members.size() * depth;
		stats.aveSize += 1.0 * map.members.size() * map.members.size();
		multimap<double, int, greater<double> > sortedMem;

		for (set<int>::iterator mem = map.members.begin(); mem != map.members.end(); mem++)
		{
			sortedMem.insert(make_pair(degree[(*mem)], (*mem)));
		}

		int i = 1;

		for (multimap<double, int, greater<double> >::iterator mem = sortedMem.begin(); mem != sortedMem.end(); mem++)
		{
			string nodeName = nodeNames[mem->second];
			string cpy_s(s + to_string(i) + " " + to_string(1.0 * mem->first / totalDegree) + " \"" + nodeName + "\"");
			DotNode* v = ir.idToNode[nodeName];
			ir.code[v] = s + to_string(i);
			i++;
		}
	}
}

void setCodeLength(IMNode **orig_node, IMNode **sub_node, int mod, TreeNode &map, double totalDegree)
{
	double exit =  sub_node[mod]->exit;
	double flow = sub_node[mod]->degree + sub_node[mod]->exit;
	double codeLength = 0.0;

	for (set<int>::iterator mem = map.members.begin(); mem != map.members.end(); mem++)
	{
		double p = orig_node[(*mem)]->degree / flow;
		codeLength -= PLOGP(p);
	}

	double p = exit / flow;
	codeLength -= PLOGP(p);
	// Weight by usage and convert to base 2
	codeLength *= flow / totalDegree / log(2.0);
	map.codeLength = codeLength;
}

void addNodesToMap(TreeNode &map, vector<double> &size)
{
	if (map.nextLevel.size() > 0)
	{
		for (multimap<double, TreeNode, greater<double> >::iterator it = map.nextLevel.begin(); it != map.nextLevel.end(); it++)
		{
			addNodesToMap(it->second, size);
		}
	}
	else
	{
		for (set<int>::iterator mem = map.members.begin(); mem != map.members.end(); mem++)
		{
			TreeNode tmp;
			tmp.members.insert((*mem));
			map.nextLevel.insert(make_pair(size[(*mem)], tmp));
		}
	}
}

void collapseTree(multimap<double, PrintTreeNode, greater<double> > &collapsedmap, TreeNode &map, vector<double> &size, int level)
{
	if ( ((level != 0) && (map.level <= level)) || (level == 0 && map.nextLevel.size() > 0))
	{
		for (multimap<double, TreeNode, greater<double> >::iterator it = map.nextLevel.begin(); it != map.nextLevel.end(); it++)
		{
			collapseTree(collapsedmap, it->second, size, level);
		}
	}
	else
	{
		PrintTreeNode tmp;
		double s = 0.0;

		for (set<int>::iterator mem = map.members.begin(); mem != map.members.end(); mem++)
		{
			s += size[(*mem)];
			tmp.members.insert(make_pair(size[(*mem)], (*mem)));
		}

		tmp.size = s;
		collapsedmap.insert(make_pair(s, tmp));
	}
}

} // namespace infomap
