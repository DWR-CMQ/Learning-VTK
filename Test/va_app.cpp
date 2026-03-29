#include "va_app.h"

App::App(const vtkSmartPointer<vtkImageData>& imageData)
{
	m_spImageData = imageData;
	m_spVAVolume = std::make_shared<VAVolume>(imageData);
	m_spVAWindow = std::make_shared<VAWindow>(500, 500, "VA", false);
}

App::~App()
{

}

void App::Init()
{
	auto spColorFunc = std::make_shared<ColorTransferFunction>();
	auto spOpacityFunc = std::make_shared<OpacityTransferfunction>();
	spColorFunc->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	spColorFunc->AddRGBPoint(-77, 0.5, 0.2, 0.2);
	spColorFunc->AddRGBPoint(94, 0.5, 0.5, 0.5);
	spColorFunc->AddRGBPoint(179, 0.9, 0.9, 0.9);
	spColorFunc->AddRGBPoint(260, 1.0, 1.0, 1.0);
	spColorFunc->AddRGBPoint(3071, 0.8, 0.7, 0.6);

	// 创建并配置不透明度传递函数，用于设置体积渲染的不透明度
	spOpacityFunc->AddPoint(-3024, 0.0);
	spOpacityFunc->AddPoint(-77, 0.0);
	spOpacityFunc->AddPoint(94, 0.29);
	spOpacityFunc->AddPoint(179, 0.55);
	spOpacityFunc->AddPoint(260, 0.84);
	spOpacityFunc->AddPoint(3071, 0.875);
	auto spVolumeProperty = std::make_shared<VAVolumeProperty>();
	spVolumeProperty->SetColorTF(0, spColorFunc);
	spVolumeProperty->SetOpacityTF(0, spOpacityFunc);
	m_spVAVolume->SetVolumeProperty(spVolumeProperty);

	double visibleBound[6];
	m_spVAVolume->ComputeVisiblePropBounds(visibleBound);

	vtkSmartPointer<vtkCamera> cameraTemp = vtkSmartPointer<vtkCamera>::New();
	m_spCamera = std::make_shared<Camera>(cameraTemp, visibleBound);
	m_spCamera->Init();

	m_spRender = std::make_shared<VARender>(m_spVAVolume, m_spCamera);
	m_spRender->Init(m_spVAWindow);
	m_spVAVolume->LoadVolume();
}


void App::Render()
{
	m_spRender->GPURender(m_spVAWindow);
	m_spVAWindow->Loop([&](float deltaTime)
	{

	});
}

void App::Update()
{

}