#pragma once

#include "imnode.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <iterator>

namespace infomap {

class Greedy
{
public:
	Greedy(int nnode, double deg, IMNode** node);
	virtual ~Greedy();
	virtual void initiate(void);
	virtual void calibrate(void);
	virtual void tune(void);
	virtual void prepare(bool sort);
	virtual void level(IMNode***, bool sort);
	virtual void move(bool &moved);
	virtual void determMove(vector<int> &moveTo);

	int Nempty;
	vector<int> mod_empty;

	vector<double> mod_exit;
	vector<double> mod_degree;
	vector<double> mod_outDegree;
	vector<int> mod_members;

	//GreedyBase
	int Nmod;
	int Nnode;

	double degree;
	double log2;

	double outDegree;
	double out_log_out;
	double exit;
	double exitDegree;
	double exit_log_exit;
	double degree_log_degree;
	double nodeDegree_log_nodeDegree;

	double indexLength;
	double moduleLength;
	double codeLength;
	double twoLevelCodeLength;

	IMNode** node;

protected:
	double plogp(double d);
	vector<pair<int, double> >::iterator link;
	vector<int> modWnode;
};

} // namespace infomap
