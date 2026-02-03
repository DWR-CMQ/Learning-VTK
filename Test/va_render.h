#pragma once

#include <vtkPolyData.h>
#include "va_shader.h"
#include "va_camera.h"
#include "va_window.h"
#include "va_volume.h"
#include "va_volume_input.h"
class VARender
{
public:
	enum BlendModes
	{
		COMPOSITE_BLEND,
		MAXIMUM_INTENSITY_BLEND,
		MINIMUM_INTENSITY_BLEND,
		AVERAGE_INTENSITY_BLEND,
		ADDITIVE_BLEND,
		ISOSURFACE_BLEND,
		SLICE_BLEND
	};

public:
	VARender(std::shared_ptr<VAVolume> spVolume, std::shared_ptr<Camera> spCamera);
    ~VARender();

	void Init(std::shared_ptr<VAWindow> spWindow);
	void GPURender(std::shared_ptr<VAWindow> spWindow);
    void RenderVolumeGeometry();
	void RenderSingleInput(std::shared_ptr<VAWindow> spWindow);
	void RendermultipleInputs();

    void BindTransformations(vtkMatrix4x4* modelViewMat);
	void FinishRendering();
private:
	void SetMapperShaderParameters(int independent, int numComp);
	void SetVolumeShaderParameters(int independent, int noOfComponents, vtkMatrix4x4* modelViewMat);
	void SetLightingShaderParameters(int numberOfSamplers);
	void SetCameraShaderParameters();
	void SetAdvancedShaderParameters(int numComp);
	void UpdateSamplingDistance();
private:
    unsigned int m_uiVao;
    unsigned int m_uiVbo;
    unsigned int m_uiEbo;

    Shader* m_pDrawShader = nullptr;
    vtkSmartPointer<vtkPolyData> BBoxPolyData;
    std::shared_ptr<VAVolume> m_spVolume;
	std::shared_ptr<VolumeInput> m_spVolumeInput;
    std::shared_ptr<Camera> m_spCamera;

	vtkNew<vtkMatrix4x4> m_mat4TempMatrix4x4;
	vtkNew<vtkMatrix4x4> m_mat4InverseProjection;
	vtkNew<vtkMatrix4x4> m_mat4InverseModelView;
	vtkNew<vtkMatrix4x4> m_mat4InverseVolume;

	int WindowLowerLeft[2];
	int WindowSize[2];

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

	int TotalNumberOfLights;
	bool DefaultLighting;

	float FinalColorWindow;
	float FinalColorLevel;

	double AverageIPScalarRange[2];
	int BlendMode;
	float ActualSampleDistance;
};