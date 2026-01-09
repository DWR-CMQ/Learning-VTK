#pragma once
#include <list>
class ColorTransferFunctionInternals;

#define VTK_CTF_RGB 0
#define VTK_CTF_HSV 1
#define VTK_CTF_LAB 2
#define VTK_CTF_DIVERGING 3
#define VTK_CTF_LAB_CIEDE2000 4
#define VTK_CTF_STEP 5

#define VTK_CTF_LINEAR 0
#define VTK_CTF_LOG10 1

template <typename T>
class VariableList 
{
public:
	std::list<T> m_list; // 核心：一个存储T类型元素的std::list
};

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

	int AddRGBPoint(double x, double r, double g, double b);
	int AddRGBPoint(double x, double r, double g, double b, double midpoint, double sharpness);
	int AddRGBPoints(double* x, double* rgbColors);
	int AddRGBPoints(double* x, double* rgbColors, double midpoint, double sharpness);
	int AddHSVPoint(double x, double h, double s, double v);
	int AddHSVPoint(double x, double h, double s, double v, double midpoint, double sharpness);
	int RemovePoint(double x);
	void RemoveAllPoints();

	void AddRGBSegment(double x1, double r1, double g1, double b1, double x2, double r2, double g2, double b2);
	void AddHSVSegment(double x1, double h1, double s1, double v1, double x2, double h2, double s2, double v2);

	void GetAboveRangeColor(double* input);
	void GetBelowRangeColor(double* input);

	void SetRange(double, double);
	void SetRange(const double rng[2]);
	int AdjustRange(double range[2]);

	void SortAndUpdateRange();
	bool UpdateRange();

	void MovePoint(double oldX, double newX);
	
	double FindMinimumXDistance();

	void GetColor(double x, double rgb[3]);
	int GetSize();

	int GetNodeValue(int index, double val[6]);
	int SetNodeValue(int index, double val[6]);
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

	double* Function;
	unsigned char* Table;

	bool IndexedLookup;
	std::list<double> AnnotatedValueList;
	std::list<double> AnnotatedValue;
	
public:
	ColorTransferFunctionInternals* Internal;

};

