#include "va_color_transferfunction.h"
#include <iostream>
#include <vector>

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