#pragma once

class ColorTransferFunctionInternals;

#define VTK_CTF_RGB 0
#define VTK_CTF_HSV 1
#define VTK_CTF_LAB 2
#define VTK_CTF_DIVERGING 3
#define VTK_CTF_LAB_CIEDE2000 4
#define VTK_CTF_STEP 5

#define VTK_CTF_LINEAR 0
#define VTK_CTF_LOG10 1

class ColorTransferFunction
{
public:
	ColorTransferFunction();
	virtual ~ColorTransferFunction();

	void GetTable(double xStart, double xEnd, int size, double* table);
	inline bool GetUseAboveRangeColor()
	{
		return UseAboveRangeColor;
	}
	inline bool GetUseBelowRangeColor()
	{
		return UseBelowRangeColor;
	}

	void GetAboveRangeColor(double* input);
	void GetBelowRangeColor(double* input);
private:
	double NanColor[3];
	int Scale;
	double Range[2];

	double AboveRangeColor[3];
	int UseAboveRangeColor;

	double BelowRangeColor[3];
	int UseBelowRangeColor;

	bool Clamping;
	int ColorSpace;
	int HSVWrap;
	int Scale;
public:
	ColorTransferFunctionInternals* Internal;

};

