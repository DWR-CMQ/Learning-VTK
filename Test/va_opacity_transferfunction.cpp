#include "va_opacity_transferfunction.h"
#include <iostream>
#include <vector>
#include <algorithm>

class PiecewiseFunctionNode
{
public:
	double X;
	double Y;
	double Sharpness;
	double Midpoint;
};

class PiecewiseFunctionCompareNodes
{
public:
	bool operator()(const PiecewiseFunctionNode* node1, const PiecewiseFunctionNode* node2)
	{
		return node1->X < node2->X;
	}
};

// A find method for finding nodes inside a specified range
class PiecewiseFunctionFindNodeInRange
{
public:
	double X1;
	double X2;
	bool operator()(const PiecewiseFunctionNode* node)
	{
		return (node->X >= this->X1 && node->X <= this->X2);
	}
};

// A find method for finding nodes outside a specified range
class PiecewiseFunctionFindNodeOutOfRange
{
public:
	double X1;
	double X2;
	bool operator()(const PiecewiseFunctionNode* node)
	{
		return (node->X < this->X1 || node->X > this->X2);
	}
};

// The internal structure for containing the STL objects
class PiecewiseFunctionInternals
{
public:
	std::vector<PiecewiseFunctionNode*> Nodes;
	PiecewiseFunctionCompareNodes CompareNodes;
	PiecewiseFunctionFindNodeInRange FindNodeInRange;
	PiecewiseFunctionFindNodeOutOfRange FindNodeOutOfRange;
	OpacityTransferfunction::SearchMethod AutomaticSearchMethod = OpacityTransferfunction::BINARY_SEARCH;
	OpacityTransferfunction::SearchMethod CustomSearchMethod = OpacityTransferfunction::BINARY_SEARCH;
	bool UseCustomSearchMethod = false;
	std::vector<PiecewiseFunctionNode*>::iterator UpperBound(PiecewiseFunctionNode* node);
	std::vector<PiecewiseFunctionNode*>::iterator InterpolationSearch(PiecewiseFunctionNode* node);
};

OpacityTransferfunction::OpacityTransferfunction()
{
	this->Clamping = 1;
	this->Range[0] = 0;
	this->Range[1] = 0;
	this->Function = nullptr;
	this->AllowDuplicateScalars = 0;
	this->UseLogScale = false;
	this->Internal = new PiecewiseFunctionInternals;
}

OpacityTransferfunction::~OpacityTransferfunction()
{
	delete[] this->Function;
	for (unsigned int i = 0; i < this->Internal->Nodes.size(); i++)
	{
		delete this->Internal->Nodes[i];
	}
	this->Internal->Nodes.clear();
	delete this->Internal;
}

void OpacityTransferfunction::Initialize()
{
	this->RemoveAllPoints();
}

int OpacityTransferfunction::AddPoint(double x, double y)
{
	return this->AddPoint(x, y, 0.5, 0.5);
}

int OpacityTransferfunction::AddPoint(double x, double y, double midpoint, double sharpness)
{
	if (midpoint < 0.0 || midpoint > 1.0)
	{
		std::cout << "Midpoint outside range [0.0, 1.0]" << std::endl;
		return -1;
	}
	if (sharpness < 0.0 || sharpness > 1.0)
	{
		std::cout << "Sharpness outside range [0.0, 1.0]" << std::endl;
		return -1;
	}

	if (!this->AllowDuplicateScalars)
	{
		this->RemovePoint(x);
	}

	PiecewiseFunctionNode* node = new PiecewiseFunctionNode;
	node->X = x;
	node->Y = y;
	node->Midpoint = midpoint;
	node->Sharpness = sharpness;

	this->Internal->Nodes.push_back(node);
	this->SortAndUpdateRange();
	
	unsigned int i;
	for (i = 0; i < this->Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->X == x && this->Internal->Nodes[i]->Y == y)
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
		retVal = -1;
	}
	return retVal;
}

void OpacityTransferfunction::SortAndUpdateRange(bool updateSearchMethod)
{

}

bool OpacityTransferfunction::UpdateRange()
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

double OpacityTransferfunction::FindMinimumXDistance()
{
	std::vector<PiecewiseFunctionNode*> const& nodes = this->Internal->Nodes;
	size_t const size = nodes.size();
	if (size < 2)
	{
		return -1.0;
	}
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

bool OpacityTransferfunction::RemovePointByIndex(size_t id)
{
	if (id > this->Internal->Nodes.size())
	{
		return false;
	}

	delete this->Internal->Nodes[id];
	this->Internal->Nodes.erase(this->Internal->Nodes.begin() + id);

	bool bModify = false;
	if (id == 0 || id == this->Internal->Nodes.size())
	{
		bModify = this->UpdateRange();
	}
	return bModify;
}

int OpacityTransferfunction::RemovePoint(double x)
{
	size_t i;
	for (i = 0; i < this->Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->X == x)
		{
			break;
		}
	}
	// If the node doesn't exist, we return -1
	if (i == this->Internal->Nodes.size())
	{
		return -1;
	}
	this->RemovePointByIndex(i);
	return static_cast<int>(i);
}

int OpacityTransferfunction::RemovePoint(double x, double y)
{
	size_t i;
	for (i = 0; i < this->Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->X == x && this->Internal->Nodes[i]->Y == y)
		{
			break;
		}
	}

	// If the node doesn't exist, we return -1
	if (i == this->Internal->Nodes.size())
	{
		return -1;
	}

	this->RemovePointByIndex(i);
	return static_cast<int>(i);
}

void OpacityTransferfunction::RemoveAllPoints()
{
	for (unsigned int i = 0; i < this->Internal->Nodes.size(); i++)
	{
		delete this->Internal->Nodes[i];
	}
	this->Internal->Nodes.clear();
	this->SortAndUpdateRange(false);
}

std::vector<PiecewiseFunctionNode*>::iterator PiecewiseFunctionInternals::UpperBound(PiecewiseFunctionNode* node)
{
	PiecewiseFunctionCompareNodes comparator;
	OpacityTransferfunction::SearchMethod searchMethod = this->AutomaticSearchMethod;

	if (this->UseCustomSearchMethod)
	{
		searchMethod = this->CustomSearchMethod;
	}

	if (searchMethod == OpacityTransferfunction::BINARY_SEARCH)
	{
		return std::upper_bound(this->Nodes.begin(), this->Nodes.end(), node, comparator);
	}
	else if (searchMethod == OpacityTransferfunction::INTERPOLATION_SEARCH)
	{
		return this->InterpolationSearch(node);
	}
	else
	{
		std::cout << "The search method should only be binary search or interpolation search" << std::endl;
		return this->Nodes.begin();
	}
}

std::vector<PiecewiseFunctionNode*>::iterator PiecewiseFunctionInternals::InterpolationSearch(PiecewiseFunctionNode* node)
{
	if (this->Nodes.empty())
	{
		return this->Nodes.end();
	}

	std::vector<PiecewiseFunctionNode*>::iterator begin = this->Nodes.begin();
	std::vector<PiecewiseFunctionNode*>::iterator end = this->Nodes.end();
	std::vector<PiecewiseFunctionNode*>::iterator mid = this->Nodes.begin();
	std::vector<PiecewiseFunctionNode*>::iterator lastNode = end - 1;

	if (node->X > (*lastNode)->X)
	{
		return this->Nodes.end();
	}
	
	bool side = true;
	while ((*begin)->X <= node->X && node->X <= (*lastNode)->X && begin != end)
	{
		double fraction = (node->X - (*begin)->X) / ((*lastNode)->X - (*begin)->X);
		mid = begin + std::iterator_traits<std::vector<PiecewiseFunctionNode*>::iterator>::difference_type(fraction * (std::distance(begin, end) - 1));
		if ((*mid)->X < node->X)
		{
			begin = mid + 1;
			side = false;
		}
		else if (node->X > (*lastNode)->X)
		{
			end = mid;
			side = true;
		}
		else
		{
			return mid;
		}
	}

	return (side == false) ? mid + 1 : mid;
}

// 将double类型的table转成float类型
void OpacityTransferfunction::GetTable(double xStart, double xEnd, int size, float* table, int stride, int logIncrements, double epsilon)
{
	double* tmpTable = new double[size];
	this->GetTable(xStart, xEnd, size, tmpTable, 1, logIncrements, epsilon);
	double* tmpPtr = tmpTable;
	float* tPtr = table;

	for (int i = 0; i < size; i++)
	{
		*tPtr = static_cast<float>(tmpPtr[i]);
		tPtr += stride;
		tmpPtr++;
	}
	delete[] tmpTable;
}

void OpacityTransferfunction::GetTable(double start, double end, int size, double* table, int stride, int logIncrements, double epsilon)
{
	int numNodes = static_cast<int>(this->Internal->Nodes.size());
	double* tptr = nullptr;
	double xLoc = 0.0;
	double xStart = start;
	double xEnd = end;

	double lastValue = 0.0;
	if (numNodes != 0)
	{
		lastValue = this->Internal->Nodes[numNodes - 1]->Y;
	}

	if (logIncrements)
	{
		xStart = std::log10(xStart);
		xEnd = std::log10(xEnd);
	}

	for (int i = 0; i < size; i++)
	{
		tptr = table + stride + i;
		if (size > 1)
		{
			xLoc = xStart + (static_cast<double>(i) / static_cast<double>(size - 1)) * (xEnd - xStart);
		}
		else
		{
			xLoc = 0.5 * (xStart + xEnd);
		}

		if (logIncrements)
		{
			xLoc = std::pow(10., xLoc);
		}

		PiecewiseFunctionNode node;
		node.X = xLoc;
		std::vector<PiecewiseFunctionNode*>::iterator lowBound;
		std::vector<PiecewiseFunctionNode*>::iterator upBound;
		upBound = this->Internal->UpperBound(&node);

		// Are we at the end? If so, just use the last value
		if (upBound == this->Internal->Nodes.end())
		{
			*tptr = this->Clamping ? lastValue : 0.0;
		}
		// Are we before the first node? If so, duplicate this nodes values
		else if (upBound == this->Internal->Nodes.begin())
		{
			*tptr = this->Clamping ? this->Internal->Nodes[0]->Y : 0.0;
		}
		else
		{
			double x1, x2, y1, y2, midpoint, sharpness;

			lowBound = upBound - 1;
			x1 = (*lowBound)->X;
			x2 = (*upBound)->X;
			y1 = (*lowBound)->Y;
			y2 = (*upBound)->Y;

			midpoint = (*lowBound)->Midpoint;
			sharpness = (*lowBound)->Sharpness;

			if (midpoint < epsilon)
			{
				midpoint = epsilon;
			}

			if (midpoint > 1 - epsilon)
			{
				midpoint = 1 - epsilon;
			}

			double scale;
			if (this->UseLogScale)
			{
				double xLog = std::log10(xLoc);
				double x1Log = std::log10(x1);
				double x2Log = std::log10(x2);
				scale = (xLog - x1Log) / (x2Log - x1Log);
			}
			else
			{
				scale = (xLoc - x1) / (x2 - x1);
			}

			if (scale < midpoint)
			{
				scale = 0.5 * scale / midpoint;
			}
			else
			{
				scale = 0.5 + 0.5 * (scale - midpoint) / (1.0 - midpoint);
			}

			if (sharpness > 0.99)
			{
				*tptr = scale < 0.5 ? y1 : y2;
			}
			// Override for sharpness < 0.01
			// In this case we want piecewise linear
			if (sharpness < 0.01)
			{
				// Simple linear interpolation
				*tptr = (1 - scale) * y1 + scale * y2;
				continue;
			}

			if (scale < 0.5)
			{
				scale = 0.5 * std::pow(scale * 2, 1.0 + 10 * sharpness);
			}
			else if (scale > 0.5)
			{
				scale = 1.0 - 0.5 * std::pow((1.0 - scale) * 2, 1 + 10 * sharpness);
			}

			double ss = scale * scale;
			double sss = ss * scale;

			double h1 = 2 * sss - 3 * ss + 1;
			double h2 = -2 * sss + 3 * ss;
			double h3 = sss - 2 * ss + scale;
			double h4 = sss - ss;

			double slope;
			double t;

			slope = y2 - y1;
			t = (1.0 - sharpness) * slope;
			*tptr = h1 * y1 + h2 * y2 + h3 * t + h4 * t;
			double min = (y1 < y2) ? y1 : y2;
			double max = (y1 > y2) ? y1 : y2;

			// Final error check to make sure we don't go outside
			// the Y range
			*tptr = (*tptr < min) ? min : *tptr;
			*tptr = (*tptr > max) ? max : *tptr;
		}
	}
}

void OpacityTransferfunction::BuildFunctionFromTable(double xStart, double xEnd, int size, double* table, int stride)
{
	double inc = 0.0;
	double* tptr = table;

	this->RemoveAllPoints();
	if (size > 1)
	{
		inc = (xEnd - xStart) / static_cast<double>(size - 1);
	}

	for (int i = 0; i < size; i++)
	{
		PiecewiseFunctionNode* node = new PiecewiseFunctionNode;
		node->X = xStart + inc * i;
		node->Y = *tptr;
		node->Sharpness = 0.0;
		node->Midpoint = 0.5;

		this->Internal->Nodes.push_back(node);
		tptr += stride;
	}
	this->SortAndUpdateRange();
}

int OpacityTransferfunction::GetSize()
{
	return static_cast<int>(this->Internal->Nodes.size());
}

const char* OpacityTransferfunction::GetType()
{
	unsigned int i;
	double value;
	double prev_value = 0.0;
	int function_type = 0;

	if (!this->Internal->Nodes.empty())
	{
		prev_value = this->Internal->Nodes[0]->Y;
	}

	for (i = 1; i < this->Internal->Nodes.size(); i++)
	{
		value = this->Internal->Nodes[i]->Y;

		// Do not change the function type if equal
		if (value != prev_value)
		{
			if (value > prev_value)
			{
				switch (function_type)
				{
				case 0:
				case 1:
					function_type = 1; // NonDecreasing
					break;
				case 2:
					function_type = 3; // Varied
					break;
				default:
					break;
				}
			}
			else
			{
				switch (function_type)
				{
				case 0:
				case 2:
					function_type = 2; // NonDecreasing
					break;
				case 1:
					function_type = 3; // Varied
					break;
				default:
					break;
				}
			}
			
		}

		prev_value = value;
		if (function_type == 3)
		{
			break;
		}
	}

	switch (function_type)
	{
	case 0:
		return "Constant";
	case 1:
		return "NonDecreasing";
	case 2:
		return "NonIncreasing";
	case 3:
		return "Varied";
	default:
		break;
	}
	return "Unknown";
}

double* OpacityTransferfunction::GetDataPointer()
{
	int size = static_cast<int>(this->Internal->Nodes.size());
	delete[] this->Function;
	this->Function = nullptr;

	if (size > 0)
	{
		this->Function = new double[size * 2];
		for (int i = 0; i < size; i++)
		{
			this->Function[2 * i] = this->Internal->Nodes[i]->X;
			this->Function[2 * i + 1] = this->Internal->Nodes[i]->Y;
		}
	}
	return this->Function;
}

void OpacityTransferfunction::FillFromDataPointer(int nb, double* ptr)
{
	if (nb <= 0 || !ptr)
	{
		return;
	}

	this->RemoveAllPoints();
	double* inPtr = ptr;
	for (int i = 0; i < nb; i++)
	{
		PiecewiseFunctionNode* node = new PiecewiseFunctionNode;
		node->X = inPtr[0];
		node->Y = inPtr[1];
		node->Sharpness = 0.0;
		node->Midpoint = 0.5;

		this->Internal->Nodes.push_back(node);
		inPtr += 2;
	}
	this->SortAndUpdateRange();
}

void OpacityTransferfunction::UpdateSearchMethod(double epsilon, double thresh)
{
	double averageDiff = 0;
	double stdDiff = 0;
	double currDiff = 0;
	const size_t nodeCount = this->Internal->Nodes.size();

	if (nodeCount < 3)
	{
		this->Internal->AutomaticSearchMethod = BINARY_SEARCH;
		return;
	}

	averageDiff = (this->Internal->Nodes[nodeCount - 1]->X - this->Internal->Nodes[0]->X) / static_cast<double>(nodeCount);
	if (std::abs(averageDiff) < epsilon)
	{
		this->Internal->AutomaticSearchMethod = BINARY_SEARCH;
		return;
	}

	for (size_t k = 0; k < this->Internal->Nodes.size() - 1; k++)
	{
		currDiff = this->Internal->Nodes[k + 1]->X - this->Internal->Nodes[k]->X;
		stdDiff += std::pow(currDiff - averageDiff, 2);
	}

	stdDiff /= std::max(static_cast<double>(this->Internal->Nodes.size() - 1), 1.0);
	stdDiff = std::sqrt(stdDiff);

	double C = std::abs(stdDiff / averageDiff);
	if (C < thresh)
	{
		this->Internal->AutomaticSearchMethod = INTERPOLATION_SEARCH;
	}
	else
	{
		this->Internal->AutomaticSearchMethod = BINARY_SEARCH;
	}
}

int OpacityTransferfunction::GetAutomaticSearchMethod()
{
	return static_cast<int>(this->Internal->AutomaticSearchMethod);
}

void OpacityTransferfunction::SetUseCustomSearchMethod(bool use)
{
	this->Internal->UseCustomSearchMethod = use;
}

void OpacityTransferfunction::SetCustomSearchMethod(int type)
{
	if (type < 0 || type >= static_cast<int>(MAX_ENUM))
	{
		type = BINARY_SEARCH;
	}
	this->Internal->CustomSearchMethod = static_cast<SearchMethod>(type);
}

int OpacityTransferfunction::GetCustomSearchMethod()
{
	return static_cast<int>(this->Internal->CustomSearchMethod);
}

int OpacityTransferfunction::EstimateMinNumberOfSamples(double const& x1, double const& x2)
{
	double const d = this->FindMinimumXDistance();
	int idealWidth = static_cast<int>(ceil((x2 - x1) / d));

	return idealWidth;
}

double OpacityTransferfunction::GetValue(double x)
{
	double table[1];
	this->GetTable(x, x, 1, table);
	return table[0];
}

int OpacityTransferfunction::AdjustRange(double range[2])
{
	if (!range)
	{
		return 0;
	}
	double* function_range = this->Range;
	if (function_range[0] < range[0])
	{
		this->AddPoint(range[0], this->GetValue(range[0]));
	}
	else
	{
		this->AddPoint(range[0], this->GetValue(function_range[0]));
	}

	if (function_range[1] > range[1])
	{
		this->AddPoint(range[1], this->GetValue(range[1]));
	}
	else
	{
		this->AddPoint(range[1], this->GetValue(function_range[1]));
	}

	// Remove all points out-of-range
	int done = 0;
	while (!done)
	{
		done = 1;
		this->Internal->FindNodeOutOfRange.X1 = range[0];
		this->Internal->FindNodeOutOfRange.X2 = range[1];

		std::vector<PiecewiseFunctionNode*>::iterator iter =
			std::find_if(this->Internal->Nodes.begin(), this->Internal->Nodes.end(), this->Internal->FindNodeOutOfRange);

		if (iter != this->Internal->Nodes.end())
		{
			delete* iter;
			this->Internal->Nodes.erase(iter);
			done = 0;
		}
	}
	this->SortAndUpdateRange();
	return 1;
}

void OpacityTransferfunction::AddSegment(double x1, double y1, double x2, double y2)
{
	// First, find all points in this range and remove them
	int done = 0;
	while (!done)
	{
		done = 1;

		this->Internal->FindNodeInRange.X1 = x1;
		this->Internal->FindNodeInRange.X2 = x2;

		std::vector<PiecewiseFunctionNode*>::iterator iter = std::find_if(
			this->Internal->Nodes.begin(), this->Internal->Nodes.end(), this->Internal->FindNodeInRange);

		if (iter != this->Internal->Nodes.end())
		{
			delete* iter;
			this->Internal->Nodes.erase(iter);
			done = 0;
		}
	}

	// Now add the points
	this->AddPoint(x1, y1, 0.5, 0.0);
	this->AddPoint(x2, y2, 0.5, 0.0);
}

int OpacityTransferfunction::GetNodeValue(int index, double val[4])
{
	int size = static_cast<int>(this->Internal->Nodes.size());
	if (index < 0 || index >= size)
	{
		std::cout << "Index out of range!" << std::endl;
		return -1;
	}

	val[0] = this->Internal->Nodes[index]->X;
	val[1] = this->Internal->Nodes[index]->Y;
	val[2] = this->Internal->Nodes[index]->Midpoint;
	val[3] = this->Internal->Nodes[index]->Sharpness;

	return 1;
}

int OpacityTransferfunction::SetNodeValue(int index, double val[4])
{
	int size = static_cast<int>(this->Internal->Nodes.size());
	if (index < 0 || index >= size)
	{
		std::cout << "Index out of range" << std::endl;
		return -1;
	}

	double oldX = this->Internal->Nodes[index]->X;
	this->Internal->Nodes[index]->X = val[0];
	this->Internal->Nodes[index]->Y = val[1];
	this->Internal->Nodes[index]->Midpoint = val[2];
	this->Internal->Nodes[index]->Sharpness = val[3];

	if (oldX != val[0])
	{
		this->SortAndUpdateRange();
	}

	return 1;
}

void OpacityTransferfunction::DeepCopy(OpacityTransferfunction* src)
{
	if (src == nullptr)
	{
		return;
	}

	// 清空当前所有控制点
	this->Clamping = src->Clamping;
	this->RemoveAllPoints();
	// 复制所有控制点
	for (int i = 0; i < src->GetSize(); i++)
	{
		double nodeValues[4];
		int isInRange = src->GetNodeValue(i, nodeValues);
		if (isInRange == 1)
		{
			// 添加控制点：x坐标, y值, 中点参数, 锐度参数
			this->AddPoint(nodeValues[0], nodeValues[1], nodeValues[2], nodeValues[3]);
		}
	}
}

void OpacityTransferfunction::ShallowCopy(OpacityTransferfunction* src)
{
	this->DeepCopy(src);
}

double OpacityTransferfunction::GetFirstNonZeroValue()
{
	if (this->Internal->Nodes.empty())
	{
		return 0;
	}

	unsigned int i;
	int all_zero = 1;
	double x = 0.0;
	for (i = 0; i < this->Internal->Nodes.size(); i++)
	{
		if (this->Internal->Nodes[i]->Y != 0.0)
		{
			all_zero = 0;
			break;
		}
	}

	// If every specified point has a zero value then return a large value
	if (all_zero)
	{
		x = std::numeric_limits<double>::max();
	}
	else
	{
		if (i > 0)
		{
			x = this->Internal->Nodes[i - 1]->X;
		}
		else
		{
			if (this->Clamping)
			{
				x = std::numeric_limits<double>::min();
			}
			else
			{
				x = this->Internal->Nodes[0]->X;
			}
		}
	}
	return x;
}