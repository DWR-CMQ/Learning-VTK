#include "va_color_transferfunction.h"
#include <iostream>
#include <vector>
#include <algorithm>


class CTFNode
{
public:
	double X;
	double R;
	double G;
	double B;
	double Sharpness;
	double Midpoint;
};

class CTFCompareNodes
{
public:
	bool operator()(const CTFNode* node1, const CTFNode* node2)
	{
		return node1->X < node2->X;
	}
};

class CTFFindNodeEqual
{
public:
	double X;
	bool operator()(const CTFNode* node)
	{
		return node->X == this->X;
	}
};

class CTFFindNodeInRange
{
public:
	double X1;
	double X2;
	bool operator()(const CTFNode* node)
	{
		return node->X >= X1 && node->X <= X2;
	}
};

class CTFFindNodeOutOfRange
{
public:
	double X1;
	double X2;
	bool operator()(const CTFNode* node)
	{
		return node->X < X1 && node->X > X2;
	}
};

class ColorTransferFunctionInternals
{
public:
	std::vector<CTFNode*> Nodes;
	CTFCompareNodes CompareNodes;
	CTFFindNodeEqual FindNodeEqual;
	CTFFindNodeInRange FindNodeInRange;
	CTFFindNodeOutOfRange FindNodeOutOfRange;
};

ColorTransferFunction::ColorTransferFunction()
{
	this->NanColor[0] = 0.5;
	this->NanColor[1] = 0.0;
	this->NanColor[2] = 0.0;
	this->Internal = new ColorTransferFunctionInternals;
	this->Range[0] = 0;
	this->Range[1] = 0;

	this->UseAboveRangeColor = 0;
	this->AboveRangeColor[0] = 1.0;
	this->AboveRangeColor[1] = 1.0;
	this->AboveRangeColor[2] = 1.0;

	this->UseBelowRangeColor = 0;
	this->BelowRangeColor[0] = 0.0;
	this->BelowRangeColor[1] = 0.0;
	this->BelowRangeColor[2] = 0.0;

	this->Clamping = 1;
	this->ColorSpace = VTK_CTF_RGB;
	this->HSVWrap = 1; // By default HSV will be wrap

	this->Scale = VTK_CTF_LINEAR;
}

ColorTransferFunction::~ColorTransferFunction()
{

}

void ColorTransferFunction::GetAboveRangeColor(double* input)
{
	input[0] = AboveRangeColor[0];
	input[1] = AboveRangeColor[1];
	input[2] = AboveRangeColor[2];
}

void ColorTransferFunction::GetBelowRangeColor(double* input)
{
	input[0] = BelowRangeColor[0];
	input[1] = BelowRangeColor[1];
	input[2] = BelowRangeColor[2];
}

int ColorTransferFunction::AddRGBPoint(double x, double r, double g, double b)
{
	return 0;
}

int ColorTransferFunction::AddRGBPoint(double x, double r, double g, double b, double midpoint, double sharpness)
{
	return 0;
}

int ColorTransferFunction::AddRGBPoints(double* x, double* rgbColors)
{
	return 0;
}

int ColorTransferFunction::AddRGBPoints(double* x, double* rgbColors, double midpoint, double sharpness)
{
	return 0;
}

int ColorTransferFunction::AddHSVPoint(double x, double h, double s, double v)
{
	return 0;
}

int ColorTransferFunction::AddHSVPoint(double x, double h, double s, double v, double midpoint, double sharpness)
{
	return 0;
}

int ColorTransferFunction::RemovePoint(double x)
{
	unsigned int i;
	for (i = 0; i < this->Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->X == x)
		{
			break;
		}
	}

	int retVal;
	if (i < this->Internal->Nodes.size())
	{
		retVal = i;
	}
	else
	{
		return -1;
	}

	this->Internal->FindNodeEqual.X = x;
	std::vector<CTFNode*>::iterator iter = std::find_if(this->Internal->Nodes.begin(), this->Internal->Nodes.end(), this->Internal->FindNodeEqual);
	if (iter != this->Internal->Nodes.end())
	{
		delete* iter;
		this->Internal->Nodes.erase(iter);
		if (i == 0 || i == this->Internal->Nodes.size())
		{
			this->UpdateRange();
		}
	}
	else
	{
		return -1;
	}
	return retVal;
}

void ColorTransferFunction::SetRange(double, double)
{

}

void ColorTransferFunction::SetRange(const double rng[2])
{

}

void ColorTransferFunction::SortAndUpdateRange()
{
	std::stable_sort(this->Internal->Nodes.begin(), this->Internal->Nodes.end(), this->Internal->CompareNodes);
	this->UpdateRange();
}

bool ColorTransferFunction::UpdateRange()
{
	double oldRange[2];
	oldRange[0] = this->Range[0];
	oldRange[1] = this->Range[1];

	int size = static_cast<int>(this->Internal->Nodes.size());
	if (size)
	{
		this->Range[0] = this->Internal->Nodes[0]->X;
		this->Range[1] = this->Internal->Nodes[size - 1]->X;
	}
	else
	{
		this->Range[0] = 0;
		this->Range[1] = 0;
	}

	if (oldRange[0] == this->Range[0] && oldRange[1] == this->Range[1])
	{
		return false;
	}
	return true;
}

void ColorTransferFunction::MovePoint(double oldX, double newX)
{
	if (oldX == newX)
	{
		return;
	}
	this->RemovePoint(newX);
	for (unsigned int i = 0; i < Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->X == oldX)
		{
			this->Internal->Nodes[i]->X == newX;
			this->SortAndUpdateRange();
			break;
		}
	}
}

void ColorTransferFunction::RemoveAllPoints()
{
	for (unsigned int i = 0; i < this->Internal->Nodes.size(); i++)
	{
		delete this->Internal->Nodes[i];
	}
	this->Internal->Nodes.clear();
	this->SortAndUpdateRange();
}

double ColorTransferFunction::FindMinimumXDistance()
{
	std::vector<CTFNode*> const& nodes = this->Internal->Nodes;
	size_t const size = nodes.size();
	double distance = std::numeric_limits<double>::max();
	for (size_t i = 0; i < size - 1; i++)
	{
		double const currentDist = nodes[i + 1]->X - nodes[i]->X;
		if (currentDist < distance)
		{
			distance = currentDist;
		}
	}
	return distance;
}

int ColorTransferFunction::AdjustRange(double range[2])
{
	if (!range)
	{
		return 0;
	}

	double* function_range = this->Range;
	double rgb[3];
	if (function_range[0] < range[0])
	{
		
	}
}

void ColorTransferFunction::GetColor(double x, double rgb[3])
{
	if (this->IndexedLookup)
	{
		int numNodes = this->GetSize();

	}
}

int ColorTransferFunction::GetSize()
{
	return static_cast<int>(this->Internal->Nodes.size());
}

int ColorTransferFunction::GetNodeValue(int index, double val[6])
{
	int size = static_cast<int>(this->Internal->Nodes.size());

	if (index < 0 || index >= size)
	{
		std::cout << "Index out of range!" << std::endl;
		return -1;
	}

	val[0] = this->Internal->Nodes[index]->X;
	val[1] = this->Internal->Nodes[index]->R;
	val[2] = this->Internal->Nodes[index]->G;
	val[3] = this->Internal->Nodes[index]->B;
	val[4] = this->Internal->Nodes[index]->Midpoint;
	val[5] = this->Internal->Nodes[index]->Sharpness;

	return 1;
}

int ColorTransferFunction::SetNodeValue(int index, double val[6])
{
	int size = static_cast<int>(this->Internal->Nodes.size());

	if (index < 0 || index >= size)
	{
		std::cout << "Index out of range!" << std::endl;
		return -1;
	}

	double oldX = this->Internal->Nodes[index]->X;
	this->Internal->Nodes[index]->X = val[0];
	this->Internal->Nodes[index]->R = val[1];
	this->Internal->Nodes[index]->G = val[2];
	this->Internal->Nodes[index]->B = val[3];
	this->Internal->Nodes[index]->Midpoint = val[4];
	this->Internal->Nodes[index]->Sharpness = val[5];

	if (oldX != val[0])
	{
		// The point has been moved, the order of points or the range might have
		// been modified.
		this->SortAndUpdateRange();
		// No need to call Modified() here because SortAndUpdateRange() has done it
		// already.
	}
	return 1;
}

void ColorTransferFunction::AddRGBSegment(double x1, double r1, double g1, double b1, double x2, double r2, double g2, double b2)
{
	int done = 0;
	while (!done)
	{
		done = 1;
		this->Internal->FindNodeInRange.X1 = x1;
		this->Internal->FindNodeInRange.X2 = x2;
		std::vector<CTFNode*>::iterator iter = std::find_if(
			this->Internal->Nodes.begin(), this->Internal->Nodes.end(), this->Internal->FindNodeInRange);

		if (iter != this->Internal->Nodes.end())
		{
			delete* iter;
			this->Internal->Nodes.erase(iter);
			done = 0;
		}
	}
	this->AddRGBPoint(x1, r1, g1, b1, 0.5, 0.0);
	this->AddRGBPoint(x2, r2, g2, b2, 0.5, 0.0);
}

void ColorTransferFunction::AddHSVSegment(double x1, double h1, double s1, double v1, double x2, double h2, double s2, double v2)
{

}

void ColorTransferFunction::GetTable(double xStart, double xEnd, int size, double* table)
{
	int i, j;
	if (std::isnan(xStart) || std::isnan(xEnd))
	{
		double* tableEntry = table;
		for (int i = 0; i < size; i++)
		{
			tableEntry[0] = this->NanColor[0];
			tableEntry[1] = this->NanColor[1];
			tableEntry[2] = this->NanColor[2];
			tableEntry += 3;
		}
		return;
	}

	int idx = 0;
	int numNodes = static_cast<int>(this->Internal->Nodes.size());
	double lastR = 0.0;
	double lastG = 0.0;
	double lastB = 0.0;
	if (numNodes != 0)
	{
		lastR = this->Internal->Nodes[numNodes - 1]->R;
		lastG = this->Internal->Nodes[numNodes - 1]->G;
		lastB = this->Internal->Nodes[numNodes - 1]->B;
	}

	double* tptr = nullptr;
	double x = 0.0;
	double x1 = 0.0;
	double x2 = 0.0;
	double rgb1[3] = { 0.0,0.0,0.0 };
	double rgb2[3] = { 0.0,0.0,0.0 };
	double midpont = 0.0;
	double sharpness = 0.0;
	
	bool usingLogScale = this->Scale == VTK_CTF_LOG10;
	if (usingLogScale)
	{
		usingLogScale = this->Range[0] > 0.0;
	}
	double logStart = 0.0;
	double logEnd = 0.0;
	double logX = 0.0;
	if (usingLogScale)
	{
		logStart = log10(xStart);
		logEnd = log10(xEnd);
	}

	for (int i = 0; i < size; i++)
	{
		tptr = table + 3 * i;
		if (size > 1)
		{
			if (usingLogScale)
			{
				logX = logStart + (static_cast<double>(i) / static_cast<double>(size - 1)) * (logEnd - logStart);
				x = pow(static_cast<double>(10.0), logX);
			}
			else
			{
				x = xStart + (static_cast<double>(i) / static_cast<double>(size - 1)) * (xEnd - xStart);
			}
		}
		else
		{
			if (usingLogScale)
			{
				logX = 0.5 * (logStart + logEnd);
				x = pow(static_cast<double>(10.0), logX);
			}
			else
			{
				x = 0.5 * (xStart + xEnd);
			}
		}

		while (idx < numNodes && x > this->Internal->Nodes[idx]->X)
		{
			idx++;
			if (idx < numNodes)
			{
				x1 = this->Internal->Nodes[idx - 1]->X;
				x2 = this->Internal->Nodes[idx]->X;
				if (usingLogScale)
				{
					x1 = log10(x1);
					x2 = log10(x2);
				}

				rgb1[0] = this->Internal->Nodes[idx - 1]->R;
				rgb2[0] = this->Internal->Nodes[idx]->R;

				rgb1[1] = this->Internal->Nodes[idx - 1]->G;
				rgb2[1] = this->Internal->Nodes[idx]->G;

				rgb1[2] = this->Internal->Nodes[idx - 1]->B;
				rgb2[2] = this->Internal->Nodes[idx]->B;

				midpont = this->Internal->Nodes[idx - 1]->Midpoint;
				sharpness = this->Internal->Nodes[idx - 1]->Sharpness;

				if (midpont < 0.00001)
				{
					midpont = 0.00001;
				}
				if (midpont > 0.99999)
				{
					midpont = 0.99999;
				}
			}
		}

		if (x > this->Range[1])
		{
			tptr[0] = 0.0;
			tptr[1] = 0.0;
			tptr[2] = 0.0;
			if (this->Clamping)
			{
				if (this->GetUseAboveRangeColor())
				{
					this->GetAboveRangeColor(tptr);
				}
				else
				{
					tptr[0] = lastR;
					tptr[1] = lastG;
					tptr[2] = lastB;
				}
			}
		}
		else if (x < this->Range[0] || std::isinf(x) && x < 0)
		{
			tptr[0] = 0.0;
			tptr[1] = 0.0;
			tptr[2] = 0.0;
			if (this->Clamping)
			{
				if (this->GetUseBelowRangeColor())
				{
					this->GetBelowRangeColor(tptr);
				}
				else
				{
					if (numNodes > 0)
					{
						tptr[0] = this->Internal->Nodes[0]->R;
						tptr[1] = this->Internal->Nodes[0]->G;
						tptr[2] = this->Internal->Nodes[0]->B;
					}
				}
			}
		}
		else if(idx == 0 && std::fabs(x - xStart) < 1e-6)
		{
			if (numNodes > 0)
			{
				tptr[0] = this->Internal->Nodes[0]->R;
				tptr[1] = this->Internal->Nodes[0]->G;
				tptr[2] = this->Internal->Nodes[0]->B;
			}
			else
			{
				tptr[0] = 0.0;
				tptr[1] = 0.0;
				tptr[2] = 0.0;
			}
		}
		else
		{
			// ²åÖµ
			double s = 0.0;
			if (usingLogScale)
			{
				s = (logX - x1) / (x2 - x1);
			}
			else
			{
				if (x2 != x1)
				{
					s = (x - x1) / (x2 - x1);
				}
			}

			if (x < midpont)
			{
				s = 0.5 * s / midpont;
			}
			else
			{
				s = 0.5 + 0.5 * (s - midpont) / (1.0 - midpont);
			}

			if (sharpness > 0.99)
			{
				if (s < 0.5)
				{
					tptr[0] = rgb1[0];
					tptr[1] = rgb1[1];
					tptr[2] = rgb1[2];
					continue;
				}
				else
				{
					tptr[0] = rgb2[0];
					tptr[1] = rgb2[1];
					tptr[2] = rgb2[2];
					continue;
				}
			}

			if (sharpness < 0.01)
			{
				if (this->ColorSpace == VTK_CTF_RGB)
				{
					tptr[0] = (1 - s) * rgb1[0] * rgb2[0];
					tptr[1] = (1 - s) * rgb1[1] * rgb2[1];
					tptr[2] = (1 - s) * rgb1[2] * rgb2[2];
				}
				else if(this->ColorSpace == VTK_CTF_STEP)
				{
					tptr[0] = rgb2[0];
					tptr[1] = rgb2[1];
					tptr[2] = rgb2[2];
				}
				else
				{
					std::cout << "ColorSpace set to invalid value" << std::endl;
				}
				continue;
			}
			if (s < 0.5)
			{
				s = 0.5 * pow(s * 2, 1.0 + 10 * sharpness);
			}
			else if (s > 0.5)
			{
				s = 1.0 - 0.5 * pow((1.0 - s) * 2, 1 + 10 * sharpness);
			}

			double ss = s * s;
			double sss = ss * s;
			double h1 = 2 * sss - 3 * ss + 1;
			double h2 = -2 * sss + 3 * ss;
			double h3 = sss - 2 * ss + s;
			double h4 = sss - ss;

			double slope;
			double t;

			if (this->ColorSpace == VTK_CTF_RGB)
			{
				for (int j = 0; j < 3; j++)
				{
					slope = rgb2[j] - rgb1[j];
					t = (1.0 - sharpness) * slope;
					tptr[j] = h1 * rgb1[j] + h2 * rgb2[j] + h3 * t + h4 * t;
				}
			}
			else if (this->ColorSpace == VTK_CTF_STEP)
			{
				tptr[0] = rgb2[0];
				tptr[1] = rgb2[1];
				tptr[2] = rgb2[2];
			}
			else
			{
				std::cout << "ColorSpace set to invalid value" << std::endl;
			}

			for (j = 0; j < 3; j++) 
			{
				tptr[j] = (tptr[j] < 0.0) ? 0.0 : tptr[j];
				tptr[j] = (tptr[j] > 1.0) ? 1.0 : tptr[j];
			}
		}
	}
}