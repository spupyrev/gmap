#pragma once

#include <vector>
#include <string>
#include <sstream>

using namespace std;

typedef vector<int> VI;
typedef vector<VI> VVI;
typedef vector<string> VS;

typedef vector<double> VD;
typedef vector<VD> VVD;

const double EPS = 1e-8;
const double INF = 123456789.0;
const double PI = 3.14159265358979323846;

template<class T> 
T Abs(const T& t)
{
	if ( t > 0 ) return t;
	return -t;
}

template<class T> 
T Sgn(const T& t)
{
	if ( t > 0 ) return 1;
	if ( t < 0 ) return -1;
	return 0;
}

template<class T> 
T Sqr2(const T& t)
{
	return ((t) * (t));
}

template <typename T> 
string to_string(const T& n)
{
	ostringstream ss;
    ss << n;
    return ss.str();
}

inline int toInt(const string& s)
{
	int n;
	istringstream (s) >> n;
	return n;
}

inline double toDouble(const string& s)
{
	double n;
	istringstream (s) >> n;
	return n;
}

vector<string> SplitNotNull(const string& s, const string& c);

double Average(const vector<double>& v);
double Median(const vector<double>& v);
double Maximum(const vector<double>& v);
double Minimum(const vector<double>& v);
double Sum(const vector<double>& v);
double Percentile(const vector<double>& v, int value);

int Compare(double numberA, double numberB);
bool Equal(double a, double b); 
bool GreaterOrEqual(double numberA, double numberB);
bool Greater(double numberA, double numberB);
bool LessOrEqual(double numberA, double numberB);
bool Less(double numberA, double numberB);
