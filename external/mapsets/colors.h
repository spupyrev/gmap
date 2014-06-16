#pragma once
#ifndef _COLORS_H_
#define _COLORS_H_

#include <cmath>

//color should be in the range [0..1]. 0 corresponds red color, 1 corresponds blue color
void SetRainbowColor(double color);

//color should be in the range [0..1]. 0 corresponds first color, 1 corresponds second color
void SetGradientColor(double color, double r1, double g1, double b1, double r2, double g2, double b2);

struct Color
{
	double r, g, b;
	double a;
	Color(): r(0), g(0), b(0), a(1) {}
	Color(double _r, double _g, double _b): r(_r), g(_g), b(_b), a(1) {}
	Color(double _r, double _g, double _b, double _a): r(_r), g(_g), b(_b), a(_a) {}

	void get(double& _r, double& _g, double& _b)
	{
		_r = r;
		_g = g;
		_b = b;
	}

	double distanse(const Color& other) const
	{
		double rmean = (r + other.r) / 2.0;
		double r = this->r - other.r;
		double g = this->g - other.g;
		double b = this->b - other.b;
		double res = (rmean + 2.0)*r*r + 4*g*g + (2.0 + (1.0 - rmean))*b*b;
		return sqrt(res);
	}

	static const Color BLACK;
	static const Color WHITE;
	static const Color BLUE;
	static const Color RED;
	static const Color GREEN;
	static const Color ORANGE;
	static const Color CYAN;
	static const Color PURPLE;
};


#endif
