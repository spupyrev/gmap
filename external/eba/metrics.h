#pragma once 
#ifndef METRICS_H_
#define METRICS_H_

#include "common.h"
#include "graph.h"

const double UNDEF = -12345.0;

struct Metrics
{
	int n, m, c;

	double mdsStress;
	double stress;
	double distortion;
	double neigPreservation;
	double compactness;
	double uniform;
	double aspectRatio;

	double modularity;
	double coverage;
	double conductance;

	Metrics()
	{
		mdsStress = UNDEF;
		stress = UNDEF;
		distortion = UNDEF;
		neigPreservation = UNDEF;
		compactness = UNDEF;
		uniform = UNDEF;
		aspectRatio = UNDEF;
		modularity = UNDEF;
		coverage = UNDEF;
		conductance = UNDEF;
	}

	void outputLayout() const
	{
		printf("|V| = %d   |E| = %d   |C| = %d\n", n, m, c);
		printf("MDS Stress:                 %s\n", safeString(mdsStress).c_str());
		printf("Graph-theoretic Stress:     %s\n", safeString(stress).c_str());
		printf("Distortion:                 %s\n", safeString(distortion).c_str());
		printf("Neighborhood Preservation:  %s\n", safeString(neigPreservation).c_str());
		//printf("Compactness:                %s\n", safeString(compactness).c_str());
		printf("Uniform Area Utilization:   %s\n", safeString(uniform).c_str());
		printf("Aspect Ratio:               %s\n", safeString(aspectRatio).c_str());
	}

	void outputCluster() const
	{
		printf("Modularity:                 %s\n", safeString(modularity).c_str());
		printf("Coverage:                   %s\n", safeString(coverage).c_str());
		printf("Conductance:                %s\n", safeString(conductance).c_str());
	}

	void output() const
	{
		outputLayout();
		outputCluster();
	}

	string safeString(double value) const
	{
		if (value == UNDEF) return "undefined";
		char s[128];
		sprintf(s, "%.4lf", value);
		return string(s);
	}
};

Metrics computeMetrics(Graph& g);

#endif
