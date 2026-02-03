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
	//m_spVAWindow->Loop([&](float deltaTime)
	//{

	//});
}

void App::Update()
{

}