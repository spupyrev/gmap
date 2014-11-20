#include "common/random_utils.h"

#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cassert>

using namespace std;

void InitRand(int seed)
{
	srand(seed);
}

void InitRand()
{
	InitRand((int)time(0));
}

int randInt()
{
	int result = rand();
	result <<= 15;
	result += rand();
	result <<= 2;
	result += rand() % 4;
	return result;
}

int randInt(int lower, int upper)
{
	assert(lower < upper);
	int result = randInt() % (upper - lower);
	if ( result < 0 ) result += upper - lower;
	return (lower + result);
}

int randInt(int upper)
{
	return randInt(0, upper);
}

char randChar()
{
	return char(randInt(32, 128));
}

char randChar(int lower, int upper)
{
	return char(randInt(lower, upper));
}

//pattern should be of form 'a..zA..Z+-='
char randChar(string pattern)
{
	vector<char> pv;
	for (int i = 0; i < (int)pattern.length(); i++)
		if ( i + 3 < (int)pattern.length() && pattern[i + 1] == '.' && pattern[i + 2] == '.' )
		{
			char beg = pattern[i];
			char end = pattern[i + 3];
			while ( beg <= end ) pv.push_back(beg++);
			i = i + 3;
		}
		else pv.push_back(pattern[i]);

	return pv[randInt(0, (int)pv.size() - 1)];
}

vector<int> randVI(int size)
{
	vector<int> result = vector<int>(size);
	for (int i = 0; i < size; i++)
		result[i] = randInt();
	return result;
}

vector<int> randVI(int size, int lower, int upper)
{
	vector<int> result = vector<int>(size);
	for (int i = 0; i < size; i++)
		result[i] = randInt(lower, upper);
	return result;
}

string randString(int length)
{
	string result;
	for (int i = 0; i < length; i++)
		result += randChar();
	return result;
}

string randString(int length, int lower, int upper)
{
	string result;
	for (int i = 0; i < length; i++)
		result += randChar(lower, upper);
	return result;
}

string randString(int length, string pattern)
{
	string result;
	for (int i = 0; i < length; i++)
		result += randChar(pattern);
	return result;
}

double randDouble()
{
	double result = (double)randInt();
	result += (double)randInt(0, 100000) / 100000.0;
	return result;
}

double randDouble(double lower, double upper)
{
	double result = randDouble();
	result = fmod(result, upper - lower);
	if ( result < 0.0 ) result += upper - lower;
	return lower + result;
}

vector<double> randVD(int size)
{
	vector<double> result = vector<double>(size);
	for (int i = 0; i < size; i++)
		result[i] = randDouble();
	return result;

}

vector<double> randVD(int size, double lower, double upper)
{
	vector<double> result = vector<double>(size);
	for (int i = 0; i < size; i++)
		result[i] = randDouble(lower, upper);
	return result;
}

int randWithProbability(const vector<double>& prob)
{
	double sum = 0;
	for (int i = 0; i < (int)prob.size(); i++)
		sum += prob[i];

	double P = randDouble(0, 1);
	double cur = 0;
	for (int i = 0; i < (int)prob.size(); i++)
	{
		double p = prob[i] / sum;
		if (p <= 1e-6) continue;
		if (cur <= P && P < cur + p) return i;
		cur += p;
	}

	return (int)prob.size() - 1;
}

vector<int> randPermutation(int size)
{
	vector<int> result = vector<int>(size);
	for (int i = 0; i < size; i++)
		result[i] = i;

	random_shuffle(result.begin(), result.end());
	return result;
}
