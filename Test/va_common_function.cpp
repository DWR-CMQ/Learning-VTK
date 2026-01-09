#include "va_common_function.h"
#include <windows.h>

bool CommonFunction::CheckDICOMDirectory(const std::string& directoryPath)
{
	// 查找目录中的文件
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA((directoryPath + "\\*").c_str(), &findFileData);

	// 如果目录无效，则返回错误信息
	if (hFind == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error: Directory does not exist or is not a directory." << std::endl;
		return false;
	}

	bool hasDICOMFiles = false;
	do
	{
		// 检查文件是否为普通文件（非目录）
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::string fileName = findFileData.cFileName;
			// 检查文件扩展名是否为 .dcm
			if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == ".dcm")
			{
				hasDICOMFiles = true;
				break;
			}
		}
	} while (FindNextFileA(hFind, &findFileData) != 0);

	// 关闭查找句柄
	FindClose(hFind);

	// 如果没有找到 DICOM 文件，则返回错误信息
	if (!hasDICOMFiles)
	{
		std::cerr << "Error: No DICOM files found in the specified directory." << std::endl;
	}

	return hasDICOMFiles;
}

void CommonFunction::HSVToRGB(double h, double s, double v, double* r, double* g, double* b)
{
	const double onethird = 1.0 / 3.0;
	const double onesixth = 1.0 / 6.0;
	const double twothird = 2.0 / 3.0;
	const double fivesixth = 5.0 / 6.0;

	// compute RGB from HSV
	if (h > onesixth && h <= onethird) // green/red
	{
		*g = 1.0;
		*r = (onethird - h) / onesixth;
		*b = 0.0;
	}
	else if (h > onethird && h <= 0.5) // green/blue
	{
		*g = 1.0;
		*b = (h - onethird) / onesixth;
		*r = 0.0;
	}
	else if (h > 0.5 && h <= twothird) // blue/green
	{
		*b = 1.0;
		*g = (twothird - h) / onesixth;
		*r = 0.0;
	}
	else if (h > twothird && h <= fivesixth) // blue/red
	{
		*b = 1.0;
		*r = (h - twothird) / onesixth;
		*g = 0.0;
	}
	else if (h > fivesixth && h <= 1.0) // red/blue
	{
		*r = 1.0;
		*b = (1.0 - h) / onesixth;
		*g = 0.0;
	}
	else // red/green
	{
		*r = 1.0;
		*g = h / onesixth;
		*b = 0.0;
	}

	// add Saturation to the equation.
	*r = (s * *r + (1.0 - s));
	*g = (s * *g + (1.0 - s));
	*b = (s * *b + (1.0 - s));

	*r *= v;
	*g *= v;
	*b *= v;
}

void CommonFunction::HSVToRGB(float h, float s, float v, float* r, float* g, float* b)
{
	double dr, dg, db;
	HSVToRGB(h, s, v, &dr, &dg, &db);
	*r = static_cast<float>(dr);
	*g = static_cast<float>(dg);
	*b = static_cast<float>(db);
}