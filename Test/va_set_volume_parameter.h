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
	void ComputeVisiblePropBounds(double allBounds[6]);
	void ComputeCellToPointMatrix(int extents[6]);
	double* GetBound();
private:
	vtkSmartPointer<vtkImageData> m_spImageData;
	float m_fCellStep[3];
	double m_dDatasetStepSize[3];
	float m_fCellSpacing[3];

	double m_dLoadedBounds[6];
	double m_dLoadedBoundsAA[6];
	int m_iExtents[6];

	int m_iIsCellData = 0;
	float AdjustedTexMin[4];
	float AdjustedTexMax[4];

	double m_dBounds[6];
	double m_dCenter[3];
	vtkMatrix4x4* Matrix;

public:
	vtkNew<vtkMatrix4x4> m_mat4TextureToDataset;
	vtkNew<vtkMatrix4x4> m_mat4TextureToDatasetInv;
	vtkNew<vtkMatrix4x4> CellToPointMatrix;
	double m_dVolumeGeometry[24];
};

