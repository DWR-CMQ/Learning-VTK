#pragma once
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrix3x3.h>
class ImageDataRelevantInfo
{
public:
	ImageDataRelevantInfo(vtkSmartPointer<vtkImageData> data);
	~ImageDataRelevantInfo();
	void Init();
	double* GetBound();
	void ComputeVisiblePropBounds(double allBounds[6]);
	
private:
	vtkSmartPointer<vtkImageData> m_spImageData;
	double m_dBounds[6];
	double m_dCenter[3];
	vtkMatrix4x4* Matrix;
};

