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

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkGPUVolumeRayCastMapper.h"
#include "vtkInteractorStyleTrackballCamera.h"
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

	// 检查读取器的输出是否有效
	vtkImageData* imageData = reader->GetOutput();
	if (!imageData)
	{
		std::cerr << "Error: Failed to read DICOM data from " << dicomDirectory << std::endl;
		return EXIT_FAILURE;
	}

	// 创建并配置颜色传递函数，用于映射标量值到颜色
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(-77, 0.5, 0.2, 0.2);
	colorTransferFunction->AddRGBPoint(94, 0.5, 0.5, 0.5);
	colorTransferFunction->AddRGBPoint(179, 0.9, 0.9, 0.9);
	colorTransferFunction->AddRGBPoint(260, 1.0, 1.0, 1.0);
	colorTransferFunction->AddRGBPoint(3071, 0.8, 0.7, 0.6);

	// 创建并配置不透明度传递函数，用于设置体积渲染的不透明度
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(-3024, 0.0);
	opacityTransferFunction->AddPoint(-77, 0.0);
	opacityTransferFunction->AddPoint(94, 0.29);
	opacityTransferFunction->AddPoint(179, 0.55);
	opacityTransferFunction->AddPoint(260, 0.84);
	opacityTransferFunction->AddPoint(3071, 0.875);

	// 创建体积属性并设置颜色和不透明度传递函数，以及其他属性
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(opacityTransferFunction);
	volumeProperty->ShadeOn();
	volumeProperty->SetInterpolationTypeToLinear();

	// 创建 GPU 体积光线投射映射器并设置输入连接
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();

	// 检查读取器的输出端口是否有效
	vtkAlgorithmOutput* outputPort = reader->GetOutputPort();
	if (!outputPort)
	{
		std::cerr << "Error: reader->GetOutputPort() returned null." << std::endl;
		return EXIT_FAILURE;
	}

	// 设置映射器的输入连接
	volumeMapper->SetInputConnection(outputPort);

	// 创建体积并设置映射器和体积属性
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	// 创建渲染器并添加体积，设置背景颜色
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddVolume(volume);
	renderer->SetBackground(0.1, 0.2, 0.3);

	// 创建渲染窗口并添加渲染器
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// 创建渲染窗口交互器并设置渲染窗口
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// 渲染场景并启动交互
	renderWindow->Render();
	renderWindowInteractor->Start();

	return 0;
}