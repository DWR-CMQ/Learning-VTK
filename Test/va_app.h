#pragma once
#include "va_window.h"
#include "va_camera.h"
#include "va_render.h"
#include "va_set_volume_parameter.h"
#include <iostream>
class App
{
public:
	App(const vtkSmartPointer<vtkImageData>& imageData);
	~App();
	void Init();
	void Render();
	void Update();
private:
	VAWindow* m_pVAWindow;
	std::shared_ptr<VARender> m_spRender;
	vtkSmartPointer<vtkImageData> m_spImageData;
	
	std::shared_ptr<SetVolumeParameter> m_spVolumePara;
	std::shared_ptr<Camera> m_spCamera;
};

