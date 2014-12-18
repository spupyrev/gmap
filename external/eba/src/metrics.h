#pragma once

#include "common/graph/dot_graph.h"

#include <fstream>

const double UNDEF = -12345.0;

class Metrics
{
public:
	int n, m, c;

	double sparseStress;
	double fullStress;
	double distortion;
	double neigPreservation;
	double uniform;
	double aspectRatio;
	double crossings;
	double minCrossAngle;
	double avgCrossAngle;

	double modularity;
	double coverage;
	double conductance;
	double contiguity;

	Metrics()
	{
		sparseStress = UNDEF;
		fullStress = UNDEF;
		distortion = UNDEF;
		neigPreservation = UNDEF;
		uniform = UNDEF;
		aspectRatio = UNDEF;
		crossings = UNDEF;
		minCrossAngle = UNDEF;
		avgCrossAngle = UNDEF;

		modularity = UNDEF;
		coverage = UNDEF;
		conductance = UNDEF;
		contiguity = UNDEF;
	}

	void OutputLayout(const string& filename) const
	{
		if (filename != "")
		{
			ofstream fileStream;
			fileStream.open(filename.c_str(), ios::out);
			OutputLayout(fileStream);
			fileStream.close();
		}
		else
		{
			OutputLayout(cout);
		}
	}

	void OutputCluster(const string& filename) const
	{
		if (filename != "")
		{
			ofstream fileStream;
			fileStream.open(filename.c_str(), ios::out);
			OutputCluster(fileStream);
			fileStream.close();
		}
		else
		{
			OutputCluster(cout);
		}
	}

	void OutputLayout(ostream& out) const
	{
		out << "|V| = " << n << "   |E| = " << m << "   |C| = " << c << "\n";

		out << "Sparse-Stress:              " << safeString(sparseStress) << "\n";
		out << "Full-Stress:                " << safeString(fullStress) << "\n";
		out << "Distortion:                 " << safeString(distortion) << "\n";
		out << "Neighborhood Preservation:  " << safeString(neigPreservation) << "\n";
		out << "Uniform Area Utilization:   " << safeString(uniform) << "\n";
		out << "Aspect Ratio:               " << safeString(aspectRatio) << "\n";
		out << "Crossings:                  " << safeString(crossings) << "\n";
		out << "Min-CrossAngle:             " << safeString(minCrossAngle) << "\n";
		out << "Avg-CrossAngle:             " << safeString(avgCrossAngle) << "\n";
	}

	void OutputCluster(ostream& out) const
	{
		out << "Modularity:                 " << safeString(modularity) << "\n";
		out << "Coverage:                   " << safeString(coverage) << "\n";
		out << "Conductance:                " << safeString(conductance) << "\n";
		out << "Contiguity:                 " << safeString(contiguity) << "\n";
	}

	void Output(const string& filename) const
	{
		OutputLayout(filename);
		OutputCluster(filename);
	}

	string safeString(double value) const
	{
		if (value == UNDEF) return "undefined";
		stringstream ss;
		ss << fixed;
		ss.precision(4);
		ss << value;
		return ss.str();
	}

	void ComputeLayout(DotGraph& g);
	void ComputeCluster(DotGraph& g);
	void Compute(DotGraph& g);
};
