#include "va_volume_visualizer.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>

VolumeVisualizer::VolumeVisualizer()
{
	m_spVolumeRenderer = vtkSmartPointer<vtkRenderer>::New();
}

VolumeVisualizer::~VolumeVisualizer()
{

}

vtkSmartPointer<vtkRenderer> VolumeVisualizer::GetRenderer()
{
	return m_spVolumeRenderer;
}

void VolumeVisualizer::DisplayVolume(vtkSmartPointer<vtkRenderWindow> renderWindow,
										vtkAlgorithmOutput* volumeData)
{
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
	// 设置映射器的输入连接
	volumeMapper->SetInputConnection(volumeData);
	
	// 创建体积并设置映射器和体积属性
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	//auto baseGPURender = volume->GetMapper();
	//auto xx = dynamic_cast<vtkOpenGLGPUVolumeRayCastMapper*>(baseGPURender);

	// 创建渲染器并添加体积，设置背景颜色
	m_spVolumeRenderer->AddVolume(volume);
	//m_spVolumeRenderer->SetViewport(0.0, 0.0, 0.5, 1.0);
	m_spVolumeRenderer->SetBackground(0.1, 0.2, 0.3);

	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	m_spVolumeRenderer->SetActiveCamera(camera);

	renderWindow->AddRenderer(m_spVolumeRenderer);
}
