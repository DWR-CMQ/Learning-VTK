#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vtkPolyData.h>
#include "va_shader.h"
#include "va_camera.h"
#include "va_set_volume_parameter.h"
class Render
{
public:
    Render(std::shared_ptr<SetVolumeParameter> spParameter, std::shared_ptr<Camera> spCamera);
    ~Render();
    void Init();
    void InitShaderInput();
private:
    unsigned int m_uiVao;
    unsigned int m_uiVbo;
    unsigned int m_uiEbo;

    Shader* m_pDrawShader = nullptr;
    vtkSmartPointer<vtkPolyData> BBoxPolyData;
    std::shared_ptr<SetVolumeParameter> m_spVolumePara;
    std::shared_ptr<Camera> m_spCamera;

	vtkNew<vtkMatrix4x4> m_mat4TempMatrix4x4;

	std::vector<float> m_vecVolMat;
	std::vector<float> m_vecInvMat;
	std::vector<float> m_vecTexMat;
	std::vector<float> m_vecInvTexMat;
	std::vector<float> m_vecTexEyeMat;
	std::vector<float> m_vecCellToPoint;
	std::vector<float> m_vecTexMin;
	std::vector<float> m_vecTexMax;
	std::vector<float> m_vecEyePos;
	std::vector<float> m_vecScale;
	std::vector<float> m_vecBias;
	std::vector<float> m_vecStep;
	std::vector<float> m_vecSpacing;
	std::vector<float> m_vecRange;
};