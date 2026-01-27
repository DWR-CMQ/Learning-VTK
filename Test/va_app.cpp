#include "va_app.h"

App::App(const vtkSmartPointer<vtkImageData>& imageData)
{
	m_spRender = std::make_shared<VARender>();
	m_spImageData = imageData;
	m_spVolumePara = std::make_shared<SetVolumeParameter>(imageData);
	m_pVAWindow = new VAWindow(800, 600, "VA", false);
}

App::~App()
{

}

void App::Init()
{
	m_spVolumePara->Init();
	double visibleBound[6];
	m_spVolumePara->ComputeVisiblePropBounds(visibleBound);

	vtkSmartPointer<vtkCamera> cameraTemp = vtkSmartPointer<vtkCamera>::New();
	m_spCamera = std::make_shared<Camera>(cameraTemp, visibleBound);
	m_spCamera->Init();
}

void App::Render()
{

}

void App::Update()
{

}