#pragma once

/// 目前采用自定义Camera包裹住vtkCamera
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include "va_imagedata_relevant_info.h"
#include <iostream>
class Camera
{
public:
	Camera(vtkSmartPointer<vtkCamera> camera, std::shared_ptr<ImageDataRelevantInfo> spVolume);
	~Camera();
	void Init();
	void ResetCameraClippingRange(const double bounds[6]);
	void ExpandBounds(double bounds[6], vtkMatrix4x4* matrix);
	void GetKeyMatrices(vtkMatrix4x4*& WCVCMatrix, vtkMatrix3x3*& normalMatrix, vtkMatrix4x4*& VCDCMatrix, vtkMatrix4x4*& WCDCMatrix);
private:
	vtkSmartPointer<vtkCamera> m_spCamera;
	std::shared_ptr<ImageDataRelevantInfo> m_spInfo;
	double ClippingRangeExpansion;
	double NearClippingPlaneTolerance;

	vtkMatrix4x4* WCDCMatrix;
	vtkMatrix4x4* WCVCMatrix;
	vtkMatrix3x3* NormalMatrix;
	vtkMatrix4x4* VCDCMatrix;
};

