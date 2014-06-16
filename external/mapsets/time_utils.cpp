#pragma warning(disable:4996)

#include "time_utils.h"

#include <ctime>
#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <string>
using namespace std;

double startTime = -1;
FILE* flog;

void InitTime()
{
	startTime = clock();
}

void InitTime(const char* logFile)
{
	InitTime();
	flog = fopen(logFile, "w");
}

void FinalizeTime()
{
	if ( startTime==-1 )
	{
		cerr<<"Time is not properly initialized\n";
		return;
	}

	OutputTimeInfo("program ends");
	if ( flog!=0 )
		fclose(flog);
}

void OutputInfo(const char* message, ...)
{
    char* buffer = new char [1024];
    
    va_list ap;
    va_start(ap, message);
    std::vsprintf(buffer, message, ap);
    va_end(ap);

	cerr<<string(buffer);
	if ( flog != 0 )
	{
		fprintf(flog, "%s", buffer);
		fflush(flog);
	}

	delete[] buffer;
}

void OutputTimeInfo(const char* message, ...)
{
    char* buffer = new char [1024];
    char* output = new char [1024];
    
    va_list ap;
    va_start(ap, message);
    std::vsprintf(buffer, message, ap);
    va_end(ap);

	sprintf(output, "%.2lf %s\n", (double)(double(clock())-startTime)/CLOCKS_PER_SEC, buffer);
	OutputInfo(output);

	delete[] buffer;
	delete[] output;
}

long CurrentTimeMillis()
{
	return clock() / (CLOCKS_PER_SEC / 1000);
}
