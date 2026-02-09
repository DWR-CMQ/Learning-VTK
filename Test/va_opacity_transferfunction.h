#pragma once

class PiecewiseFunctionInternals;
class OpacityTransferfunction
{
public:
	OpacityTransferfunction();
	virtual ~OpacityTransferfunction();

	void Initialize();
	int AddPoint(double x, double y);
	int AddPoint(double x, double y, double midpoint, double sharpness);

	bool RemovePointByIndex(size_t id);
	int RemovePoint(double x);
	int RemovePoint(double x, double y);
	void RemoveAllPoints();

	void GetTable(double x1, double x2, int size, float* table, int stride = 1, int logIncrements = 0, double epsilon = 1e-5);
	void GetTable(double x1, double x2, int size, double* table, int stride = 1, int logIncrements = 0, double epsilon = 1e-5);

	void BuildFunctionFromTable(double xStart, double xEnd, int size, double* table, int stride = 1);

	int GetSize();
	const char* GetType();

	double* GetDataPointer();
	void FillFromDataPointer(int nb, double* ptr);

	void UpdateSearchMethod(double epsilon = 1e-12, double thresh = 1e-4);
	int GetAutomaticSearchMethod();
	void SetUseCustomSearchMethod(bool use);
	void SetCustomSearchMethod(int type);
	int GetCustomSearchMethod();

	int EstimateMinNumberOfSamples(double const& x1, double const& x2);
	int AdjustRange(double range[2]);
	double GetValue(double x);
	void AddSegment(double x1, double y1, double x2, double y2);

	void GetRange(double value[2]);
	double* GetRange();
	void GetRange(double& value1, double& value2);

	// 外部给this深拷贝
	void DeepCopy(OpacityTransferfunction* src);
	// 外部给this前拷贝
	void ShallowCopy(OpacityTransferfunction* src);

	int GetNodeValue(int index, double val[4]);
	int SetNodeValue(int index, double val[4]);

	double GetFirstNonZeroValue();

	enum SearchMethod
	{
		BINARY_SEARCH = 0,
		INTERPOLATION_SEARCH = 1,
		MAX_ENUM = 2
	};

private:
	void SortAndUpdateRange(bool updateSearchMethod = true);
	bool UpdateRange();
	double FindMinimumXDistance();
	PiecewiseFunctionInternals* Internal;
	int Clamping;
	double* Function;
	double Range[2];
	int AllowDuplicateScalars;
	bool UseLogScale;

private:
	OpacityTransferfunction(const OpacityTransferfunction&) = delete;
	void operator=(const OpacityTransferfunction&) = delete;
};

