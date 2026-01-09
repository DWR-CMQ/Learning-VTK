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
};