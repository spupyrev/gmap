#pragma once 
#pragma warning (disable:4996) 

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
const double INF = 123456789.0;

template<class T> T Abs(const T& t)
{
	if ( t > 0 ) return t;
	return -t;
}

template<class T> T Sgn(const T& t)
{
	if ( t > 0 ) return 1;
	if ( t < 0 ) return -1;
	return 0;
}

template<class T> T Sqr2(const T& t)
{
	return ((t)*(t));
}

VS splitNotNull(string s, string c);
int string2Int(string s);
string int2String(int i);
string double2String(double i);

double AverageValue(const vector<double>& v);
double MedianValue(const vector<double>& v);
double MaximumValue(const vector<double>& v);
double MinimumValue(const vector<double>& v);
double Sum(const vector<double>& v);
int ChooseRandomWithProbability(VD& prob);

