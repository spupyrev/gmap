#include "common/graph/dot_graph.h"

#include <algorithm>
#include <queue>
#include <functional>

void DotGraph::initShortestPaths(DotNode* s, bool weighted)
{
	const double INF = 123456789.0;

	VD dist(nodes.size(), INF);
	dist[s->index] = 0;

	typedef pair<double, int> QE;

	priority_queue<QE, vector<QE>, greater<QE> > q;
	q.push(make_pair(0.0, s->index));

	while (!q.empty())
	{
		QE now = q.top();
		q.pop();

		if (now.first <= dist[now.second])
		{
			int v = now.second;
			for (int i = 0; i < (int)adj[v].size(); i++)
			{
				int next = adj[v][i];
				DotEdge* edge = edges[adjE[v][i]];
				assert(edge != NULL);

				double cost = (weighted ? edge->getLen() : 1.0);
				if (dist[next] > dist[v] + cost)
				{
					dist[next] = dist[v] + cost;
					q.push(make_pair(dist[next], next));
				}
			}
		}
	}

	for (int i = 0; i < (int)nodes.size(); i++)
	{
		pair<DotNode*, DotNode*> pair = make_pair(s, nodes[i]);
		double d = (dist[nodes[i]->index] < INF ? dist[nodes[i]->index] : -1);
		if (weighted)
			shortestPathsW[pair] = d;
		else
			shortestPaths[pair] = (int)d;
	}
}

vector<ConnectedDotGraph> DotGraph::getConnectedComponents()
{
	map<DotNode*, int> color;
	for (int i = 0; i < (int)nodes.size(); i++)
		color[nodes[i]] = -1;

	vector<ConnectedDotGraph> result;
	for (int i = 0; i < (int)nodes.size(); i++)
	{
		DotNode* s = nodes[i];
		if (color[s] != -1) continue;
		color[s] = (int)result.size();

		vector<DotNode*> nc;

		//bfs
		queue<DotNode*> q;
		q.push(s);

		while (!q.empty())
		{
			DotNode* now = q.front();
			q.pop();
			nc.push_back(now);

			for (int j = 0; j < (int)adj[now->index].size(); j++)
			{
				DotNode* next = nodes[adj[now->index][j]];
				if (color[next] != -1) continue;

				color[next] = color[s];
				q.push(next);
			}
		}

		result.push_back(ConnectedDotGraph(*this, nc));
	}

	sort(result.begin(), result.end());
	reverse(result.begin(), result.end());

	return result;
}
