#include "vtkRenderingCoreModule.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkGPUVolumeRayCastMapper.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkSmartPointer.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageData.h"
#include "vtkAutoInit.h"
#include "vtkVersion.h"
#include "vtkCamera.h"
#include <iostream>
#include <windows.h>
#include <gl/GL.h>

#include "va_mesh_visualizer.h"
#include "va_dicom2mesh.h"
#include "va_volume_visualizer.h"

// 链接 OpenGL 库
#pragma comment(lib, "opengl32.lib")

// 初始化 VTK 所需的模块
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
VTK_MODULE_INIT(vtkRenderingUI)

// 函数定义：用于检查给定目录中是否包含 DICOM 文件
bool CheckDICOMDirectory(const std::string& directoryPath)
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
	// 输出 VTK 版本信息
	std::cout << "VTK Version: " << vtkVersion::GetVTKSourceVersion() << std::endl;
	std::string dicomDirectory = "F:/DicomDataSet/Circle of Willis";
	if (!CheckDICOMDirectory(dicomDirectory))
	{
		std::cout << "CheckDICOMDirectory Failed" << std::endl;
		return EXIT_FAILURE;
	}

	vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(dicomDirectory.c_str());
	reader->Update();

	vtkImageData* imageData = reader->GetOutput();
	if (!imageData)
	{
		std::cerr << "Error: Failed to read DICOM data from " << dicomDirectory << std::endl;
	}

	auto spDicom2Mesh = std::make_shared<Dicom2mesh>();
	auto spMesh3D = spDicom2Mesh->DicomToMesh(imageData, 0.0, true, 100.0);

	auto spRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	spRenderWindow->SetSize(1000, 500);

	auto spVolumeVisa = std::make_shared<VolumeVisualizer>();
	auto spMeshVisa = std::make_shared<MeshVisualizer>();
	spVolumeVisa->DisplayVolume(spRenderWindow, reader->GetOutputPort());
	spMeshVisa->DisplayMesh(spRenderWindow, spMesh3D);

	auto spVolumeRender = spVolumeVisa->GetRenderer();
	auto spMeshRender = spMeshVisa->GetRenderer();
	auto volumeRenderCamera = spVolumeRender->GetActiveCamera();
	if (volumeRenderCamera == nullptr)
	{
		std::cerr << "Error volumeRenderCamera is null" << std::endl;
		return 0;
	}
	spMeshRender->SetActiveCamera(volumeRenderCamera);

	spVolumeRender->ResetCamera();
	spVolumeRender->ResetCameraClippingRange();
	spMeshRender->ResetCameraClippingRange();

	// 创建渲染窗口交互器并设置渲染窗口
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(spRenderWindow);

	// 创建交互样式并设置给渲染窗口交互器
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// 渲染场景并启动交互
	spRenderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}
