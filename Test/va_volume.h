#pragma once
#include <iostream>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include "va_texture_object.h"
#include "va_volume_property.h"
class VAVolume
{
public:
	VAVolume(const vtkSmartPointer<vtkImageData>& imageData);
	~VAVolume();

	void LoadVolume();
	void Init();
	void ComputeBounds();
	void ComputeVisiblePropBounds(double allBounds[6]);
	void ComputeCellToPointMatrix(int extents[6]);
	double* GetBound();
	void GetScaleAndBias(int scalarType, float* scalarRange, float& scale, float& bias);
	void SelectTextureFormat(unsigned int& format, unsigned int& internalFormat, int& type,
		int scalarType, int noOfComponents);
	std::shared_ptr<VAVolumeProperty> GetVolumeProperty();
private:
	vtkSmartPointer<vtkImageData> m_spImageData;
	std::shared_ptr<TextureObject> m_spVolumeTexture;
	std::shared_ptr<VAVolumeProperty> m_spVolumeProperty;
	double m_dDatasetStepSize[3];

	double m_dLoadedBounds[6];
	double m_dLoadedBoundsAA[6];
	int m_iExtents[6];

	int m_iIsCellData = 0;

	double m_dBounds[6];
	double m_dCenter[3];
	vtkMatrix4x4* Matrix;

public:
	vtkNew<vtkMatrix4x4> m_mat4TextureToDataset;
	vtkNew<vtkMatrix4x4> m_mat4TextureToDatasetInv;
	vtkNew<vtkMatrix4x4> CellToPointMatrix;
	float AdjustedTexMin[4];
	float AdjustedTexMax[4];
	float Scale[4];
	float Bias[4];
	float ScalarRange[4][2];

	float m_fCellSpacing[3];
	float m_fCellStep[3];

	bool HandleLargeDataTypes;
	double m_dVolumeGeometry[24];
};

