#pragma once
#include <iostream>
#include <string>
#include <vector>

class CommonFunction
{
public:
	static bool CheckDICOMDirectory(const std::string& directoryPath);

	static void HSVToRGB(double h, double s, double v, double* r, double* g, double* b);
	static void HSVToRGB(float h, float s, float v, float* r, float* g, float* b);
	static void HSVToRGB(const double hsv[3], double rgb[3]);
	static void HSVToRGB(const float hsv[3], float rgb[3]);

	static void RGBToHSV(const float rgb[3], float hsv[3]);
	static void RGBToHSV(float r, float g, float b, float* h, float* s, float* v);
	static void RGBToHSV(const double rgb[3], double hsv[3]);
	static void RGBToHSV(double r, double g, double b, double* h, double* s, double* v);
};