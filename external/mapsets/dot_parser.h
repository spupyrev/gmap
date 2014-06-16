#pragma once 

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
		bool insideQuote = false;
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
				if (c == ';' && !insideQuote)
				{
					if (s.length() > 0)
						res.push_back(s);
					s = "";
				}
				else 
				{
					if (c == '"') insideQuote = !insideQuote;
					s += c;
				}
			}
		}

		return res;
	}

	bool isSpecial(const string& line)
	{
		string beforeBrakets, insideBrackets;
		splitLine(line, beforeBrakets, insideBrackets);
		string s = trim(beforeBrakets);

		if (s == "node") return true;
		if (s == "graph") return true;
		if (s == "edge") return true;
		return false;
	}

	bool isNode(const string& line)
	{
		string beforeBrakets, insideBrackets;
		splitLine(line, beforeBrakets, insideBrackets);

		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j+1] == '-') return false;

		return true;
	}

	bool isEdge(const string& line)
	{
		string beforeBrakets, insideBrackets;
		splitLine(line, beforeBrakets, insideBrackets);

		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j+1] == '-') return true;

		return false;
	}

	Edge* parseEdge(const string& line, int index)
	{		   
		string beforeBrakets, insideBrackets;
		splitLine(line, beforeBrakets, insideBrackets);

		Edge* e = new Edge(index);
		//ids
		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j+1] == '-')
			{
				string n1 = beforeBrakets.substr(0, j);
				string n2 = beforeBrakets.substr(j+2, beforeBrakets.length() - j - 2);
				e->s = extractId(n1);
				e->t = extractId(n2);
			}
		//attrs
		e->attr = parseAttr(insideBrackets);
		return e;
	}

	Node* parseNode(const string& line, int index)
	{
		string beforeBrakets, insideBrackets;
		splitLine(line, beforeBrakets, insideBrackets);

		Node* n = new Node(index);
		n->id = extractId(beforeBrakets);
		n->attr = parseAttr(insideBrackets);
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

	map<string, string> parseAttr(const string& line)
	{
		string s = trim(line);

		VS tmp = splitAttr(s, ',');
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
		VS tmp = splitAttr(line, '=');
		assert(tmp.size() == 2);
		key = trim(tmp[0]);
		value = extractId(tmp[1]);
	}

	void splitLine(const string& line, string& beforeBrakets, string& insideBrackets)
	{
		beforeBrakets = insideBrackets = "";

		int i = 0;
		bool insideQuote = false;
		while (i < (int)line.length()) 
		{
			if (line[i] == '[' && !insideQuote) break;
			if (line[i] == '"') insideQuote = !insideQuote;
			i++;
		}
		beforeBrakets = line.substr(0, i);
		assert(!insideQuote);

		int j = i;
		while (j < (int)line.length()) 
		{
			if (line[j] == ']' && !insideQuote) break;
			if (line[j] == '"') insideQuote = !insideQuote;
			j++;
		}
		if (j-i-1 > 0) insideBrackets = line.substr(i+1, j-i-1);
	}

	VS splitAttr(const string& line, char separator)
	{
		VS res;
		bool insideQuotes = false;
		string s = "";
		for (int i = 0; i < (int)line.length(); i++)
		{
			if (!insideQuotes)
			{
				if (line[i] == '"')
				{
					insideQuotes = true;
					s += line[i];
					continue;
				}

				if (line[i] == separator)
				{
					if (s.length() > 0) res.push_back(s);
					s = "";
					continue;
				}

				s += line[i];
			}
			else
			{
				if (line[i] == '"') insideQuotes = false;
				s += line[i];
			}
		}

		if (s.length() > 0) res.push_back(s);

		return res;
	}

	string trim(const string& line)
	{
		if (line.length() == 0) return line;

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

		int j = (int)line.length() - 1;
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

};

struct DotWriter
{
	void writeGraph(const string& filename, const Graph& g)
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
