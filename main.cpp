//#include <vtkActor.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkCellPicker.h>
//#include <vtkImagePlaneWidget.h>
//#include <vtkMarchingCubes.h>
//#include <vtkImageCast.h>
//#include <vtkVolumeRayCastCompositeFunction.h>
//#include <vtkVolumeProperty.h>
//#include <vtkVolume.h>
//#include <vtkVolumeRayCastMapper.h>
//#include <vtkPiecewiseFunction.h>
//#include <vtkColorTransferFunction.h>
//#include <vtkGPUVolumeRayCastMapper.h>
//#include <vtkImageMapToColors.h>
//#include <vtkProperty.h>
//#include <vtkImageActor.h>
//#include<vtkImageData.h>
//#include<vtkVolumeRayCastCompositeFunction.h>
//#include<vtkGPUVolumeRayCastMapper.h>
//#include<vtkSmartPointer.h>

#include "vtkSmartPointer.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageData.h"
#include <iostream>
#include <windows.h>

// 函数定义：用于检查给定目录中是否包含 DICOM 文件
bool checkDICOMDirectory(const std::string& directoryPath)
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

int main(int argc, char* argv[])
{

	// 获取 DICOM 目录路径
	std::string dicomDirectory = "F:/DicomDataSet/Circle of Willis";

	// 检查 DICOM 目录中是否包含 DICOM 文件
	if (!checkDICOMDirectory(dicomDirectory))
	{
		return EXIT_FAILURE;
	}

	// 创建 DICOM 图像读取器并设置 DICOM 目录
	vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(dicomDirectory.c_str());
	reader->Update();

	int imageDims[3];
	reader->GetOutput()->GetDimensions(imageDims); //need include <vtkimagedata.h>
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;



	return 0;
}