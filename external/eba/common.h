#pragma once 
#pragma warning (disable:4996) 
#ifndef COMMON_H_
#define COMMON_H_

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cmath>

using namespace std;

typedef vector<int> VI;
typedef vector<VI> VVI;
typedef vector<string> VS;

typedef vector<double> VD;
typedef vector<VD> VVD;

const double EPS = 1e-6;

#define Abs(a) ((a) >= 0 ? (a) : -(a))

VS splitNotNull(string s, string c);
int string2Int(string s);
string int2String(int i);

double AverageValue(const vector<double>& v);
double MedianValue(const vector<double>& v);
double MaximumValue(const vector<double>& v);
double MinimumValue(const vector<double>& v);
double Sum(const vector<double>& v);
int ChooseRandomWithProbability(VD& prob);

#endif
