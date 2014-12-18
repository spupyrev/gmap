#pragma once

#include "common/graph/dot_graph.h"

#include <iostream>
#include <fstream>

class DotReader
{
	DotReader(const DotReader&);
	DotReader& operator = (const DotReader&);

public:
	DotReader() {}

	DotGraph ReadGraph(const string& filename)
	{
		VS lines;

		if (filename != "")
		{
			ifstream fileStream;
			fileStream.open(filename.c_str(), ios::in);
			if (!fileStream)
			{
				cerr << "can't open input file '" << filename << "'" << endl;
				throw 1;
			}
			lines = ReadLines(fileStream);
			fileStream.close();
		}
		else
		{
			lines = ReadLines(cin);
		}

		DotGraph g;
		for (int i = 0; i < (int)lines.size(); i++)
		{
			if (isSpecial(lines[i]))
			{
				DotNode* v = ParseNode(lines[i], -1);
				g.style.push_back(v);
			}
			else if (isEdge(lines[i]))
			{
				DotEdge* e = ParseEdge(lines[i], g.edges.size());
				g.edges.push_back(e);
			}
			else if (isNode(lines[i]))
			{
				DotNode* v = ParseNode(lines[i], g.nodes.size());
				g.nodes.push_back(v);
			}
			else
			{
				cerr << "Unknown entry: " << lines[i] << "\n";
			}
		}

		g.initAdjacencyList();
		return g;
	}

private:
	VS ReadLines(istream& is)
	{
		VS res;
		char c;
		string s = "";
		int state = -1;
		bool insideQuote = false;
		while (is.get(c))
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
		SplitLine(line, beforeBrakets, insideBrackets);
		string s = trim(beforeBrakets);

		if (s == "node") return true;
		if (s == "graph") return true;
		if (s == "edge") return true;
		return false;
	}

	bool isNode(const string& line)
	{
		string beforeBrakets, insideBrackets;
		SplitLine(line, beforeBrakets, insideBrackets);

		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j + 1] == '-') return false;

		return true;
	}

	bool isEdge(const string& line)
	{
		string beforeBrakets, insideBrackets;
		SplitLine(line, beforeBrakets, insideBrackets);

		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j + 1] == '-') return true;

		return false;
	}

	DotEdge* ParseEdge(const string& line, int index)
	{
		string beforeBrakets, insideBrackets;
		SplitLine(line, beforeBrakets, insideBrackets);

		DotEdge* e = new DotEdge(index);
		//ids
		for (int j = 0; j + 1 < (int)beforeBrakets.length(); j++)
			if (beforeBrakets[j] == '-' && beforeBrakets[j + 1] == '-')
			{
				string n1 = beforeBrakets.substr(0, j);
				string n2 = beforeBrakets.substr(j + 2, beforeBrakets.length() - j - 2);
				e->s = ExtractId(n1);
				e->t = ExtractId(n2);
			}
		//attrs
		e->attr = ParseAttr(insideBrackets);
		return e;
	}

	DotNode* ParseNode(const string& line, int index)
	{
		string beforeBrakets, insideBrackets;
		SplitLine(line, beforeBrakets, insideBrackets);

		DotNode* n = new DotNode(index);
		n->id = ExtractId(beforeBrakets);
		n->attr = ParseAttr(insideBrackets);
		return n;
	}

	string ExtractId(const string& line)
	{
		string s = trim(line);
		if (s[0] == '"')
		{
			assert(s[s.length() - 1] == '"');
			return s.substr(1, s.length() - 2);
		}

		return s;
	}

	map<string, string> ParseAttr(const string& line)
	{
		string s = trim(line);

		VS tmp = SplitAttr(s, ',');
		map<string, string> attr;
		for (int i = 0; i < (int)tmp.size(); i++)
		{
			string key, value;
			ExtractAttr(tmp[i], key, value);
			attr[key] = value;
		}
		return attr;
	}

	void ExtractAttr(const string& line, string& key, string& value)
	{
		VS tmp = SplitAttr(line, '=');
		assert(tmp.size() == 2);
		key = trim(tmp[0]);
		value = ExtractId(tmp[1]);
	}

	void SplitLine(const string& line, string& beforeBrakets, string& insideBrackets)
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
		if (j - i - 1 > 0) insideBrackets = line.substr(i + 1, j - i - 1);
	}

	VS SplitAttr(const string& line, char separator)
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
		return line.substr(i, j - i + 1);
	}

};

class DotWriter
{
	DotWriter(const DotWriter&);
	DotWriter& operator = (const DotWriter&);

public:
	DotWriter() {}

	void WriteGraph(const string& filename, const DotGraph& g)
	{
		if (filename != "")
		{
			ofstream fileStream;
			fileStream.open(filename.c_str(), ios::out);
			WriteGraph(fileStream, g);
			fileStream.close();
		}
		else
		{
			WriteGraph(cout, g);
		}
	}

private:
	void WriteGraph(ostream& os, const DotGraph& g)
	{
		os << "graph {\n";
		WriteStyles(os, g);
		WriteNodes(os, g);
		WriteEdges(os, g);
		os << "}\n";
	}

	void WriteStyles(ostream& os, const DotGraph& g)
	{
		for (int i = 0; i < (int)g.style.size(); i++)
			WriteStyle(os, g.style[i], false);
	}

	void WriteNodes(ostream& os, const DotGraph& g)
	{
		for (int i = 0; i < (int)g.nodes.size(); i++)
			WriteNode(os, g.nodes[i], true);
	}

	void WriteStyle(ostream& os, const DotNode* n, bool useQ)
	{
		if (useQ)
			os << "  \"" << n->id << "\" ";
		else
			os << "  " << n->id << " ";

		WriteAttr(os, n->attr);
		os << ";\n";
	}

	void WriteNode(ostream& os, const DotNode* n, bool useQ)
	{
		if (useQ)
			os << "  \"" << n->id << "\" ";
		else
			os << "  " << n->id << " ";

		if (n->attr.size() > 0)
			WriteAttr(os, n->attr);
		os << ";\n";
	}

	void WriteAttr(ostream& os, const map<string, string>& attr)
	{
		os << "[";
		for (map<string, string>::const_iterator iter = attr.begin(); iter != attr.end(); iter++)
		{
			string key = (*iter).first;
			string value = (*iter).second;
			if (iter != attr.begin()) os << ", ";
			os << key << "=\"" << value << "\"";
		}

		os << "]";
	}

	void WriteEdges(ostream& os, const DotGraph& g)
	{
		for (int i = 0; i < (int)g.edges.size(); i++)
			WriteEdge(os, g.edges[i]);
	}

	void WriteEdge(ostream& os, const DotEdge* n)
	{
		os << "  \"" << n->s << "\" -- \"" << n->t << "\" ";
		if (n->attr.size() > 0)
			WriteAttr(os, n->attr);
		os << ";\n";
	}

};
