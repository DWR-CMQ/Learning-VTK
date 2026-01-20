#pragma once
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vector>
class SetVolumeParameter
{
public:
	SetVolumeParameter(const vtkSmartPointer<vtkImageData>& imageData);
	~SetVolumeParameter();

	void Init();
	void ComputeBounds();
	void CalculateParameter();

	void ComputeVisiblePropBounds(double allBounds[6]);
	double* GetBound();
private:
	vtkSmartPointer<vtkImageData> m_spImageData;
	float m_fCellStep[3];
	double m_dDatasetStepSize[3];
	float m_fCellSpacing[3];

	double m_dLoadedBounds[6];
	double m_dLoadedBoundsAA[6];
	double m_dVolumeGeometry[24];
	int m_iExtents[6];

	int m_iIsCellData = 0;

	vtkNew<vtkMatrix4x4> m_mat4TextureToDataset;
	vtkNew<vtkMatrix4x4> m_mat4TextureToDatasetInv;
	vtkNew<vtkMatrix4x4> m_mat4TempMatrix4x4;

	double m_dBounds[6];
	double m_dCenter[3];
	vtkMatrix4x4* Matrix;

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

