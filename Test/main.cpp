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
#include "vtkRendererCollection.h"
#include "vtkVolumeCollection.h"
#include "vtkOpenGLGPUVolumeRayCastMapper.h"
#include "vtkLight.h"
#include <iostream>
#include <windows.h>
#include <gl/GL.h>

#include "va_mesh_visualizer.h"
#include "va_dicom2mesh.h"
#include "va_volume_visualizer.h"
#include "va_app.h"
#include "va_common_function.h"
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

	//auto spDicom2Mesh = std::make_shared<Dicom2mesh>();
	//auto spMesh3D = spDicom2Mesh->DicomToMesh(imageData, 0.0, true, 100.0);

	auto spRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	spRenderWindow->SetSize(500, 500);

	auto spVolumeVisa = std::make_shared<VolumeVisualizer>();
	//auto spMeshVisa = std::make_shared<MeshVisualizer>();
	spVolumeVisa->DisplayVolume(spRenderWindow, reader->GetOutputPort());
	//spMeshVisa->DisplayMesh(spRenderWindow, spMesh3D);

	auto spVolumeRender = spVolumeVisa->GetRenderer();
	//auto spMeshRender = spMeshVisa->GetRenderer();
	auto volumeRenderCamera = spVolumeRender->GetActiveCamera();
	if (volumeRenderCamera == nullptr)
	{
		std::cerr << "Error volumeRenderCamera is null" << std::endl;
		return 0;
	}
	//spMeshRender->SetActiveCamera(volumeRenderCamera);

	spVolumeRender->ResetCamera();
	spVolumeRender->ResetCameraClippingRange();
	//spMeshRender->ResetCameraClippingRange();

	// 创建渲染窗口交互器并设置渲染窗口
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(spRenderWindow);

	// 创建交互样式并设置给渲染窗口交互器
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// 渲染场景并启动交互
	//spRenderWindow->Render();
    //renderWindowInteractor->Start();

    App app(imageData);
    app.Init();
    app.Render();

    //auto renderColl = spRenderWindow->GetRenderers();
    //auto firstRender = renderColl->GetFirstRenderer();
    //auto volumeColl = firstRender->GetVolumes();
    //auto currentVolume = volumeColl->GetVolume();
    //auto baseGPURender = currentVolume->GetMapper();
    //auto openglGPURender = dynamic_cast<vtkOpenGLGPUVolumeRayCastMapper*>(baseGPURender);
    //auto internalImpl = openglGPURender->GetImpl();

    //// uniform变量
    //auto volumeMatrix = internalImpl->VolMatVec;
    //auto inverseVolumeMatrix = internalImpl->InvMatVec;
    //auto textureDatasetMatrix = internalImpl->TexMatVec;
    //auto inverseTextureDatasetMatrix = internalImpl->InvTexMatVec;
    //auto textureToEye = internalImpl->TexEyeMatVec;
    //auto texMin = internalImpl->TexMinVec;
    //auto texMax = internalImpl->TexMaxVec;
    //auto eyePosObjs = internalImpl->EyePosVec;
    //auto cellToPoint = internalImpl->CellToPointVec;

    //vtkOpenGLCamera* cam = vtkOpenGLCamera::SafeDownCast(volumeRenderCamera);
    //vtkMatrix4x4* glTransformMatrix;
    //vtkMatrix4x4* modelViewMatrix;
    //vtkMatrix3x3* normalMatrix;
    //vtkMatrix4x4* projectionMatrix;
    //cam->GetKeyMatrices(spVolumeRender, modelViewMatrix, normalMatrix, projectionMatrix, glTransformMatrix);

    //auto volume_scale = internalImpl->ScaleVec.data();
    //auto volume_bias = internalImpl->BiasVec.data();
    //auto scalarsRange = internalImpl->RangeVec.data();
    //auto cellStep = internalImpl->StepVec.data();
    //auto cellSpacing = internalImpl->SpacingVec.data();
    //auto sampleDistance = internalImpl->ActualSampleDistance;
    //float windowLowerLeftCorner[2];
    //internalImpl->ToFloat(internalImpl->WindowLowerLeft, windowLowerLeftCorner);

    //float inverseOriginalWindowSize[2];
    //internalImpl->ToFloat(1.0 / internalImpl->WindowSize[0], 1.0 / internalImpl->WindowSize[1], inverseOriginalWindowSize);

    //float inverseWindowSize[2];
    //internalImpl->ToFloat(1.0 / internalImpl->WindowSize[0], 1.0 / internalImpl->WindowSize[1], inverseWindowSize);

    //// in_textureExtentsMax和in_textureExtentsMin
    //auto& input = internalImpl->Parent->AssembledInputs[0];
    //auto volumeTex = input.Texture.GetPointer();
    //vtkVolumeTexture::VolumeBlock* block = volumeTex->GetCurrentBlock();
    //auto blockExt = block->Extents;
    //float textureExtentsMax[3];
    //internalImpl->ToFloat(blockExt[0], blockExt[2], blockExt[4], textureExtentsMax);
    //float textureExtentsMin[3];
    //internalImpl->ToFloat(blockExt[1], blockExt[3], blockExt[5], textureExtentsMin);

    //// 光照参数
    //auto vol = input.Volume;
    //const int independent = vol->GetProperty()->GetIndependentComponents();
    //// noOfComponents
    //const int numComp = volumeTex->GetLoadedScalars()->GetNumberOfComponents();
    //int const numSamplers = (independent ? numComp : 1);
    //auto volumeProperty = vol->GetProperty();
    //float ambient[4][3];
    //float diffuse[4][3];
    //float specular[4][3];
    //float specularPower[4];

    //for (int i = 0; i < numSamplers; ++i)
    //{
    //    ambient[i][0] = ambient[i][1] = ambient[i][2] = volumeProperty->GetAmbient(i);
    //    diffuse[i][0] = diffuse[i][1] = diffuse[i][2] = volumeProperty->GetDiffuse(i);
    //    specular[i][0] = specular[i][1] = specular[i][2] = volumeProperty->GetSpecular(i);
    //    specularPower[i] = volumeProperty->GetSpecularPower(i);
    //}

    //// averageIPRange
    //double avgRange[2];
    //float averageIPRange[2];
    //internalImpl->Parent->GetAverageIPScalarRange(avgRange);
    //if (avgRange[1] < avgRange[0])
    //{
    //    double tmp = avgRange[1];
    //    avgRange[1] = avgRange[0];
    //    avgRange[0] = tmp;
    //}
    //internalImpl->ToFloat(avgRange[0], avgRange[1], averageIPRange);

    //// twoSidedLighting
    //bool twoSidedLighting = spVolumeRender->GetTwoSidedLighting();

    //// lightAmbientColor && lightDiffuseColor && lightSpecularColor
    //vtkLightCollection* lc = spVolumeRender->GetLights();
    //vtkLight* light;
    //vtkCollectionSimpleIterator sit;
    //// those light parameters are used by both positional and directional lights
    //std::vector<std::array<float, 3>> lightAmbientColor(internalImpl->TotalNumberOfLights);
    //std::vector<std::array<float, 3>> lightDiffuseColor(internalImpl->TotalNumberOfLights);
    //std::vector<std::array<float, 3>> lightSpecularColor(internalImpl->TotalNumberOfLights);
    //std::vector<std::array<float, 3>> lightDirection(internalImpl->TotalNumberOfLights);
    //int idxPositional = 0;
    //int idxDirectional = internalImpl->NumberPositionalLights;
    //int idxLight = 0;
    //for (lc->InitTraversal(sit); (light = lc->GetNextLight(sit));)
    //{
    //    float status = light->GetSwitch();
    //    if (status > 0.0)
    //    {
    //        idxLight = light->GetPositional() ? idxPositional : idxDirectional;
    //        double* aColor = light->GetAmbientColor();
    //        double* dColor = light->GetDiffuseColor();
    //        double* sColor = light->GetSpecularColor();
    //        double intensity = light->GetIntensity();
    //        for (int i = 0; i < 3; i++)
    //        {
    //            lightAmbientColor[idxLight][i] = aColor[i] * intensity;
    //            lightDiffuseColor[idxLight][i] = dColor[i] * intensity;
    //            lightSpecularColor[idxLight][i] = sColor[i] * intensity;
    //        }
    //    }
    //}

    //auto in_scale = 1.0 / internalImpl->Parent->GetFinalColorWindow();
    //auto in_bias = 0.5 - (internalImpl->Parent->GetFinalColorLevel() / internalImpl->Parent->GetFinalColorWindow());

	return EXIT_SUCCESS;
}
