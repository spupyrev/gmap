#include "common.h"

VS splitNotNull(string s, string c)
{
	s += c;
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

int string2Int(string s)
{
	int result = 0;
	sscanf(s.c_str(), "%d", &result);
	return result;
}

double Sum(const vector<double>& v)
{
	double sum = 0;
	for (int i = 0; i < (int)v.size(); i++)
		sum += v[i];
	return sum;
}

double AverageValue(const vector<double>& v)
{
	double av = Sum(v);
	if (!v.empty())
		av /= (double)v.size();
	return av;
}

double MedianValue(const vector<double>& v)
{
	if ( v.empty() ) return 0;
	if ( v.size() == 1 ) return v[0];

	vector<double> tmp = v;
	sort(tmp.begin(), tmp.end());
	int sz = (int)tmp.size();

	if ( sz%2 == 0 )
		return (tmp[sz/2 - 1] + tmp[sz/2])/2.0;
	else
		return tmp[sz/2];
}

double MaximumValue(const vector<double>& v)
{
	if ( v.empty() ) return 0;

	double res = v[0];
	for (int i = 0; i < (int)v.size(); i++)
		res = max(res, v[i]);
	return res;
}

double MinimumValue(const vector<double>& v)
{
	if ( v.empty() ) return 0;

	double res = v[0];
	for (int i = 0; i < (int)v.size(); i++)
		res = min(res, v[i]);
	return res;
}

int ChooseRandomWithProbability(VD& prob)
{
	double sum = Sum(prob);
	int rnd = rand()%100000;
	double P = (double)rnd/100000.0;
	double cur = 0;
	for (int i = 0; i < (int)prob.size(); i++)
	{
		double p = prob[i]/sum;
		if (p <= 1e-6) continue;
		if (cur <= P && P < cur + p) return i;
		cur += p;
	}

	return (int)prob.size() - 1;
}
