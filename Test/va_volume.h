#pragma once
#include <iostream>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkTuple.h>
#include "va_texture_object.h"
#include "va_volume_property.h"
class VAVolume
{
	typedef vtkTuple<int, 6> Size6;
	typedef vtkTuple<int, 3> Size3;
public:
	VAVolume(const vtkSmartPointer<vtkImageData>& imageData);
	~VAVolume();

	void LoadVolume();
	void ComputeVisiblePropBounds(double allBounds[6]);
	double* GetBound();
	int* GetExtent();
	void GetScaleAndBias(int scalarType, float* scalarRange, float& scale, float& bias);

	std::shared_ptr<VAVolumeProperty> GetVolumeProperty();
	vtkDataArray* GetLoadedScalars();
	vtkSmartPointer<vtkImageData> GetImageData();

	void CreateBlocks(unsigned int format, unsigned int internalFormat, int type);
	void LoadTexture(int interpolation);
private:
	void ComputeCellToPointMatrix(int extents[6]);
	void UpdateTextureToDataMatrix();
	void ComputeBounds();
	void SelectTextureFormat(unsigned int& format, unsigned int& internalFormat, int& type,
		int scalarType, int noOfComponents);
	Size3 ComputeBlockSize(int* extent);

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
	vtkDataArray* Scalars;

	std::vector<Size3> TextureSizes;
	Size3 TextureSize;
	Size6 FullExtent;
	Size3 FullSize;
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
	int InterpolationType;

	bool HandleLargeDataTypes;
	double m_dVolumeGeometry[24];
};

