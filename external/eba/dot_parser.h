#pragma once 
#ifndef DOTPARSER_H_
#define DOTPARSER_H_

#include "common.h"
#include "graph.h"

struct DotParser
{
	Graph parseGraph(const string& filename)
	{
		Graph g;
		VS lines = ReadLines(filename);

		for (int i = 0; i < (int)lines.size(); i++)
		{
			//cerr<<lines[i]<<"\n";

			if (isSpecial(lines[i]))
			{
				Node* v = parseNode(lines[i], -1);
				g.style.push_back(v);
			}
			else if (isEdge(lines[i]))
			{
				Edge* e = parseEdge(lines[i], g.edges.size());
				g.edges.push_back(e);
			}
			else if (isNode(lines[i]))
			{
				Node* v = parseNode(lines[i], g.nodes.size());
				g.nodes.push_back(v);
			}
			else
			{
				cerr<<"Unknown entry: "<<lines[i]<<"\n";
			}
		}

		g.initAdjacencyList();
		return g;
	}

	VS ReadLines(const string& filename)
	{
		VS res;

		if (filename != "")
			freopen(filename.c_str(), "r", stdin);

		char c;
		string s = "";
		int state = -1;
		while ((c = getchar()) != EOF)
		{
			if (c == '{')
			{
				state = 0;
				continue;
			}
			if (c == '}')
			{
				state = -1;
				continue;
			}

			if (state == 0)
			{
				if (c == ';')
				{
					if (s.length() > 0)
						res.push_back(s);
					s = "";
				}
				else 
					s += c;
			}
		}

		return res;
	}

	bool isSpecial(const string& line)
	{
		VS tmp = splitNotNull(line, "[] \n\t\r");
		if (tmp[0] == "node") return true;
		if (tmp[0] == "graph") return true;
		if (tmp[0] == "edge") return true;
		return false;
	}

	bool isNode(const string& line)
	{
		int i = 0;
		while (i < (int)line.length() && line[i] != '[') i++;
		string s1 = line.substr(0, i);

		for (int j = 0; j + 1 < (int)s1.length(); j++)
			if (s1[j] == '-' && s1[j+1] == '-') return false;

		return true;
	}

	bool isEdge(const string& line)
	{
		int i = 0;
		while (i < (int)line.length() && line[i] != '[') i++;
		string s1 = line.substr(0, i);

		for (int j = 0; j + 1 < (int)s1.length(); j++)
			if (s1[j] == '-' && s1[j+1] == '-') return true;

		return false;
	}

	Edge* parseEdge(const string& line, int index)
	{		   
		Edge* e = new Edge(index);
		int i = 0;
		while (i < (int)line.length() && line[i] != '[') i++;
		string s1 = line.substr(0, i);

		int j = i;
		while (j < (int)line.length() && line[j] != ']') j++;
		if (j-i-1 > 0) e->attr = parseAttr(line.substr(i+1, j-i-1));

		for (int j = 0; j + 1 < (int)s1.length(); j++)
			if (s1[j] == '-' && s1[j+1] == '-')
			{
				string n1 = s1.substr(0, j);
				string n2 = s1.substr(j+2, s1.length()-j-2);
				e->s = extractId(n1);
				e->t = extractId(n2);
			}

		return e;
	}

	Node* parseNode(const string& line, int index)
	{
		Node* n = new Node(index);
		int i = 0;
		while (i < (int)line.length() && line[i] != '[') i++;
		string s1 = line.substr(0, i);

		int j = i;
		while (j < (int)line.length() && line[j] != ']') j++;
		if (j-i-1 > 0) n->attr = parseAttr(line.substr(i+1, j-i-1));

		n->id = extractId(s1);
		return n;
	}

	string extractId(const string& line)
	{
		string s = trim(line);
		if (s[0] == '"')
		{
			assert(s[s.length()-1] == '"');
			return s.substr(1, s.length() - 2);
		}

		return s;
	}

	string trim(const string& line)
	{
		int i = 0;
		while (i < (int)line.length())
		{
			if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' || line[i] == '\r') 
			{
				i++;
				continue;
			}
			break;
		}

		int j = (int)line.length()-1;
		while (j >= 0)
		{
			if (line[j] == ' ' || line[j] == '\n' || line[j] == '\t' || line[j] == '\r') 
			{
				j--;
				continue;
			}
			break;
		}

		assert(i <= j);
		return line.substr(i, j-i+1);
	}

	map<string, string> parseAttr(const string& line)
	{
		VS tmp = splitAttr(line);
		map<string, string> attr;
		for (int i = 0; i < (int)tmp.size(); i++)
		{
			string key, value;
			extractAttr(tmp[i], key, value);
			attr[key] = value;
		}
		return attr;
	}

	void extractAttr(const string& line, string& key, string& value)
	{
		VS tmp = splitNotNull(line, "=");
		assert(tmp.size() == 2);
		key = trim(tmp[0]);
		value = extractId(tmp[1]);
	}

	VS splitAttr(const string& line)
	{
		VS res;
		int state = 0;
		string s = "";
		for (int i = 0; i < (int)line.length(); i++)
		{
			if (state == 0)
			{
				if (line[i] == '"')
				{
					state = 1;
					s += line[i];
					continue;
				}

				if (line[i] == ',')
				{
					if (s.length() > 0) res.push_back(s);
					s = "";
					continue;
				}

				s += line[i];
			}
			else
			{
				//inside ""
				if (line[i] == '"')
				{
					state = 0;
				}

				s += line[i];
			}
		}

		if (s.length() > 0) res.push_back(s);

		return res;
	}
};

struct DotWriter
{
	void writeGraph(string& filename, const Graph& g)
	{
		if (filename != "")
			freopen(filename.c_str(), "w", stdout);

		printf("graph {\n");
		writeStyles(g);
		writeNodes(g);
		writeEdges(g);
		printf("}\n");
	}

	void writeStyles(const Graph& g)
	{
		for (int i = 0; i < (int)g.style.size(); i++)
			writeNode(g.style[i], false);
	}

	void writeNodes(const Graph& g)
	{
		for (int i = 0; i < (int)g.nodes.size(); i++)
			writeNode(g.nodes[i], true);
	}

	void writeNode(const Node* n, bool useQ)
	{
		if (useQ)
			printf("  \"%s\" ", n->id.c_str());
		else
			printf("  %s ", n->id.c_str());

		if (n->attr.size() > 0)
			writeAttr(n->attr);
		printf(";\n");
	}

	void writeAttr(const map<string, string>& attr)
	{
		printf("[");
		for (map<string, string>::const_iterator iter = attr.begin(); iter != attr.end(); iter++)
		{
			string key = (*iter).first;
			string value = (*iter).second;
			if (iter != attr.begin()) printf(", ");
			printf("%s=\"%s\"", key.c_str(), value.c_str());
		}

		printf("]");
	}

	void writeEdges(const Graph& g)
	{
		for (int i = 0; i < (int)g.edges.size(); i++)
			writeEdge(g.edges[i]);
	}

	void writeEdge(const Edge* n)
	{
		printf("  \"%s\" -- \"%s\" ", n->s.c_str(), n->t.c_str());
		if (n->attr.size() > 0)
			writeAttr(n->attr);
		printf(";\n");
	}

};

#endif
