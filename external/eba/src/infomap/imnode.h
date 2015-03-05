#pragma once

#include <vector>
using namespace std;

namespace infomap {

class IMNode
{
public:
	IMNode() {}
	IMNode(int modulenr)
	{
		index = modulenr;
		exit = 0.0;
		degree = 0.0;
		outDegree = 0.0;
		members.push_back(modulenr);
	}

	IMNode(IMNode* oldNode)
	{
		index = oldNode->index;
		exit = oldNode->exit;
		outDegree = oldNode->outDegree;
		degree = oldNode->degree;
		int Nmembers = oldNode->members.size();
		members = vector<int>(Nmembers);

		for (int i = 0; i < Nmembers; i++)
			members[i] = oldNode->members[i];

		int Nlinks = oldNode->links.size();
		links = vector<pair<int, double> >(Nlinks);

		for (int i = 0; i < Nlinks; i++)
		{
			links[i].first = oldNode->links[i].first;
			links[i].second = oldNode->links[i].second;
		}
	}

	~IMNode()
	{
		members.clear();
		links.clear();
	}

	vector<int> members; // If module, lists member nodes in module
	vector<pair<int, double> > links; // List of identities and link weight of connected nodes/modules

	double exit; // total weight of links to other nodes / modules
	double degree; // total degree of node / module
	double outDegree; // total weight of links to nodes / modules outside the branch
	int index; // the node / module identity
};

} // namespace infomap
