#include "common/common.h"

#include <algorithm>

VS SplitNotNull(const string& ss, const string& c)
{
	string s = ss + c;
	VS result;
	string tec = "";
	for (int i = 0; i < (int)s.length(); i++)
	{
		if (c.find(s[i]) != string::npos)
		{
			if ((int)tec.length() > 0) result.push_back(tec);
			tec = "";
		}
		else tec += s[i];
	}

	return result;
}

double Sum(const vector<double>& v)
{
	double sum = 0;
	for (int i = 0; i < (int)v.size(); i++)
		sum += v[i];
	return sum;
}

double Average(const vector<double>& v)
{
	double av = Sum(v);
	if (!v.empty())
		av /= (double)v.size();
	return av;
}

double Median(const vector<double>& v)
{
	if ( v.empty() ) return 0;
	if ( v.size() == 1 ) return v[0];

	vector<double> tmp = v;
	sort(tmp.begin(), tmp.end());
	int sz = (int)tmp.size();

	if ( sz % 2 == 0 )
		return (tmp[sz / 2 - 1] + tmp[sz / 2]) / 2.0;
	else
		return tmp[sz / 2];
}

double Maximum(const vector<double>& v)
{
	if ( v.empty() ) return 0;

	double res = v[0];
	for (int i = 0; i < (int)v.size(); i++)
		res = max(res, v[i]);
	return res;
}

double Minimum(const vector<double>& v)
{
	if (v.empty()) return 0;

	double res = v[0];
	for (int i = 0; i < (int)v.size(); i++)
		res = min(res, v[i]);
	return res;
}

double Percentile(const vector<double>& v, int p)
{
	if (v.empty()) return 0;

	int n = (int)v.size();
	int pos = p * n / 100;
	if (pos >= n) pos = n - 1;
	return v[pos];
}

int Compare(double numberA, double numberB) 
{
    double c = numberA - numberB;
    if (c <= -EPS)
        return -1;
    if (c >= EPS)
        return 1;
    return 0;
}

bool Equal(double a, double b) 
{
    return Abs(a - b) <= EPS;
}

bool Greater(double numberA, double numberB) 
{
    return Compare(numberA, numberB) > 0;
}

bool GreaterOrEqual(double numberA, double numberB) 
{
    return Compare(numberA, numberB) >= 0;
}

bool LessOrEqual(double numberA, double numberB) 
{
    return Compare(numberA, numberB) <= 0;
}

bool Less(double numberA, double numberB) 
{
    return Compare(numberA, numberB) < 0;
}
