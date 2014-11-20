#include "common/common.h"
#include "common/graph/dot_graph.h"
#include "common/graph/dot_parser.h"

int main()
{
	DotReader parser;
	DotGraph g = parser.ReadGraph("");

	//remove background
	for (int i = 0; i < (int)g.style.size(); i++)
	{
		if (g.style[i]->id == "graph")
		{
			g.style[i]->removeAttr("_background");
			g.style[i]->removeAttr("bb");
			g.style[i]->removeAttr("bgcolor");
			//g.style.erase(g.style.begin() + i);
			break;
		}
	}

	//remove shape
	//set style to fillcolor
	for (int i = 0; i < (int)g.style.size(); i++)
	{
		if (g.style[i]->id == "node")
		{
			g.style[i]->removeAttr("shape");
			g.style[i]->setAttr("style", "filled");
			break;
		}
	}

	//remove edges
	//g.edges.erase(g.edges.begin(), g.edges.end());

	//add fillcolor
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		string clr = g.nodes[i]->getAttr("clustercolor");
		g.nodes[i]->setAttr("fillcolor", clr);
	}

	DotWriter writer;
	writer.WriteGraph("", g);
	return 0;
}
