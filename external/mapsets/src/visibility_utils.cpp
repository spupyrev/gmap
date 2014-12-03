#include "visibility_utils.h"

vector<Segment> GetBoundaryObstacles(const DotGraph& g, const string& avoidClusterId, double marginCoef)
{
	vector<Segment> obstacles;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		DotNode* node = g.nodes[i];
		if (node->getAttr("cluster") == avoidClusterId) continue;

		vector<Segment> boundary = node->getBoundary(marginCoef);
		for (int j = 0; j < (int)boundary.size(); j++)
			obstacles.push_back(boundary[j]);
	}

	return obstacles;
}

vector<Segment> GetTreeObstacles(const map<string, SegmentSet*>& trees, const string& avoidClusterId)
{
	vector<Segment> result;
	for (auto iter = trees.begin(); iter != trees.end(); iter++)
	{
		if ((*iter).first == avoidClusterId) continue;

		SegmentSet* tree = (*iter).second;
		for (int i = 0; i < tree->count(); i++)
		{
			result.push_back(tree->get(i));
		}
	}

	return result;
}

