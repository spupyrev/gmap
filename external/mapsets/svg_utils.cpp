#include "svg_utils.h"

void writeSVGHead(FILE* f)
{
	fprintf(f, "<?xml version='1.0' encoding='utf-8' standalone='yes'?>\n");
	fprintf(f, "<svg version = '1.1'\n");
	fprintf(f, "	xmlns = 'http://www.w3.org/2000/svg'\n");
	fprintf(f, "	xmlns:xlink = 'http://www.w3.org/1999/xlink'\n");
	fprintf(f, "	xmlns:ev = 'http://www.w3.org/2001/xml-events'\n");
	fprintf(f, "	height = '100%%'  width = '100%%'>\n");
}

void writeSVGTail(FILE* f)
{
	fprintf(f, "</svg>\n");
}

string rgb(Color color)
{
	char s[128];
	int r = int(color.r * 255.0);
	int g = int(color.g * 255.0);
	int b = int(color.b * 255.0);
	sprintf(s, "rgb(%d,%d,%d)", r, g, b);
	return string(s);
}

void drawSVGCircle(FILE* f, Point p, double r, Color c)
{
	p.Scale(SVG_SCALE);
	fprintf(f, "	<circle cx='%.3lf' cy='%.3lf' r='%.3lf' fill='%s'/>\n", p.x, -p.y, r, rgb(c).c_str());
}

void drawSVGLine(FILE* f, Point p1, Point p2, Color c, double width)
{
	p1.Scale(SVG_SCALE);
	p2.Scale(SVG_SCALE);
	fprintf(f, "	<line x1='%.3lf' y1='%.3lf' x2='%.3lf' y2='%.3lf' style='stroke:%s;stroke-width:%.3lf;stroke-opacity:%.3lf;fill:none'/>\n", p1.x, -p1.y, p2.x, -p2.y, rgb(c).c_str(), width, c.a);
}

void drawSVGLine(FILE* f, Point p1, Point p2, Color c)
{
	drawSVGLine(f, p1, p2, c, 0.1);
}

void drawSVGText(FILE* f, Point p, const char* text)
{
	p.Scale(SVG_SCALE);
	fprintf(f, "	<text x='%.3lf' y='%.3lf' style='font-family:Verdana;font-size:1' >%s</text>\n", p.x, p.y, text);
}
