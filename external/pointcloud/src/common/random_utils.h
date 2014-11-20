#pragma once

#include <vector>
#include <string>
using namespace std;

void InitRand(int seed);
void InitRand();

int randInt();
int randInt(int lower, int upper);
int randInt(int upper);

char randChar();
char randChar(int lower, int upper);
//pattern should be of form 'a..zA..Z+-='
char randChar(string pattern);

vector<int> randVI(int size);
vector<int> randVI(int size, int lower, int upper);

string randString(int length);
string randString(int length, int lower, int upper);
string randString(int length, string pattern);

double randDouble();
double randDouble(double lower, double upper);

vector<double> randVD(int size);
vector<double> randVD(int size, double lower, double upper);

int randWithProbability(const vector<double>& prob);

vector<int> randPermutation(int size);
