#pragma once

#include "point.h"
#include "colors.h"

#include <cstdio>

using namespace std;

#define SVG_SCALE 1.0

void writeSVGHead(FILE* f);
void writeSVGTail(FILE* f);

string rgb(Color color);

void drawSVGCircle(FILE* f, Point p, double r, Color c);
void drawSVGLine(FILE* f, Point p1, Point p2, Color c, double width);
void drawSVGLine(FILE* f, Point p1, Point p2, Color c);
void drawSVGText(FILE* f, Point p, const char* text);
