#pragma once 
#ifndef METRICS_H_
#define METRICS_H_

#include "common.h"
#include "graph.h"

const double UNDEF = -12345.0;

struct Metrics
{
	int n, m, c;

	double sparseStress;
	double fullStress;
	double distortion;
	double neigPreservation;
	double compactness;
	double uniform;
	double aspectRatio;
	double crossings;
	double minCrossAngle;
	double avgCrossAngle;

	double modularity;
	double coverage;
	double conductance;

	Metrics()
	{
		sparseStress = UNDEF;
		fullStress = UNDEF;
		distortion = UNDEF;
		neigPreservation = UNDEF;
		compactness = UNDEF;
		uniform = UNDEF;
		aspectRatio = UNDEF;
		crossings = UNDEF;
		minCrossAngle = UNDEF;
		avgCrossAngle = UNDEF;

		modularity = UNDEF;
		coverage = UNDEF;
		conductance = UNDEF;
	}

	void outputLayout() const
	{
		printf("|V| = %d   |E| = %d   |C| = %d\n", n, m, c);
		printf("Sparse-Stress:              %s\n", safeString(sparseStress).c_str());
		printf("Full-Stress:                %s\n", safeString(fullStress).c_str());
		printf("Distortion:                 %s\n", safeString(distortion).c_str());
		printf("Neighborhood Preservation:  %s\n", safeString(neigPreservation).c_str());
		//printf("Compactness:                %s\n", safeString(compactness).c_str());
		printf("Uniform Area Utilization:   %s\n", safeString(uniform).c_str());
		printf("Aspect Ratio:               %s\n", safeString(aspectRatio).c_str());
		printf("Crossings:                  %s\n", safeString(crossings).c_str());
		printf("Min-CrossAngle:             %s\n", safeString(minCrossAngle).c_str());
		printf("Avg-CrossAngle:             %s\n", safeString(avgCrossAngle).c_str());
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
