#pragma once 
#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

void InitTime();
void InitTime(const char* logFile);
void FinalizeTime();
void OutputInfo(const char* message, ...);
void OutputTimeInfo(const char* message, ...);
long CurrentTimeMillis();

#endif
