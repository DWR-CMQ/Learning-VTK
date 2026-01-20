#include "va_set_volume_parameter.h"
#include <vtkMath.h>
SetVolumeParameter::SetVolumeParameter(const vtkSmartPointer<vtkImageData>& imageData)
{
	m_spImageData = imageData;
    m_mat4TempMatrix4x4->Identity();
    ComputeBounds();
    Matrix = vtkMatrix4x4::New();
}

SetVolumeParameter::~SetVolumeParameter()
{
    // 矩阵析构
    this->Matrix->Delete();
    this->Matrix = nullptr;
}

void SetVolumeParameter::Init()
{
	double origin[3];
	double spacing[3];
	vtkMatrix3x3* directionMat = vtkMatrix3x3::New();
	directionMat->Identity();
	if (m_spImageData != nullptr)
	{
		directionMat->DeepCopy(m_spImageData->GetDirectionMatrix()->GetData());
		m_spImageData->GetOrigin(origin);
		m_spImageData->GetSpacing(spacing);
	}

    auto stepsize = this->m_dDatasetStepSize;
    vtkMatrix4x4* matrix = this->m_mat4TextureToDataset;
    matrix->Identity();
    double* result = matrix->GetData();

    double* direction = directionMat->GetData();
    for (int i = 0; i < 3; i++)
    {
        result[i * 4] = direction[i * 3] / stepsize[0];
        result[i * 4 + 1] = direction[i * 3 + 1] / stepsize[0];
        result[i * 4 + 2] = direction[i * 3 + 2] / stepsize[0];
    }

    double blockOrigin[3];
    vtkImageData::TransformContinuousIndexToPhysicalPoint(this->m_iExtents[0], this->m_iExtents[2], this->m_iExtents[4],
                                        origin, spacing, direction, blockOrigin);
    result[3] = blockOrigin[0];
    result[7] = blockOrigin[1];
    result[11] = blockOrigin[2];

    auto matrixInv = this->m_mat4TextureToDatasetInv.GetPointer();
    matrixInv->DeepCopy(matrix);
    matrixInv->Invert();
    directionMat->Delete();
}

void SetVolumeParameter::ComputeBounds()
{
    if (m_spImageData == nullptr)
    {
        std::cout << "ComputeBounds m_spImageData is nulltpr" << std::endl;
        return;
    }

    double spacing[3];
    double origin[3];
    double* direction = nullptr;

    m_spImageData->GetSpacing(spacing); /// TODO could be causing inf issue on streaming
    m_spImageData->GetExtent(this->m_iExtents);
    m_spImageData->GetOrigin(origin);
    direction = m_spImageData->GetDirectionMatrix()->GetData();
    
    int swapBounds[3];
    swapBounds[0] = (spacing[0] < 0);
    swapBounds[1] = (spacing[1] < 0);
    swapBounds[2] = (spacing[2] < 0);

    // push corners through matrix to get bounding box
    int iMin, iMax, jMin, jMax, kMin, kMax;
    int* extent = this->m_iExtents;
    iMin = extent[0];
    iMax = extent[1] + this->m_iIsCellData;
    jMin = extent[2];
    jMax = extent[3] + this->m_iIsCellData;
    kMin = extent[4];
    kMax = extent[5] + this->m_iIsCellData;
    int ijkCorners[8][3] = { { iMin, jMin, kMin }, { iMax, jMin, kMin }, { iMin, jMax, kMin },
      { iMax, jMax, kMin }, { iMin, jMin, kMax }, { iMax, jMin, kMax }, { iMin, jMax, kMax },
      { iMax, jMax, kMax } };
    double xMin, xMax, yMin, yMax, zMin, zMax;
    xMin = yMin = zMin = VTK_DOUBLE_MAX;
    xMax = yMax = zMax = VTK_DOUBLE_MIN;
    for (int i = 0; i < 8; ++i)
    {
        int* ijkCorner = ijkCorners[i];
        double* xyz = this->m_dVolumeGeometry + i * 3;

        vtkImageData::TransformContinuousIndexToPhysicalPoint(
            ijkCorner[0], ijkCorner[1], ijkCorner[2], origin, spacing, direction, xyz);
        
        if (xyz[0] < xMin)
            xMin = xyz[0];
        if (xyz[0] > xMax)
            xMax = xyz[0];
        if (xyz[1] < yMin)
            yMin = xyz[1];
        if (xyz[1] > yMax)
            yMax = xyz[1];
        if (xyz[2] < zMin)
            zMin = xyz[2];
        if (xyz[2] > zMax)
            zMax = xyz[2];
    }
    this->m_dLoadedBoundsAA[0] = xMin;
    this->m_dLoadedBoundsAA[1] = xMax;
    this->m_dLoadedBoundsAA[2] = yMin;
    this->m_dLoadedBoundsAA[3] = yMax;
    this->m_dLoadedBoundsAA[4] = zMin;
    this->m_dLoadedBoundsAA[5] = zMax;

    // If spacing is negative, we may have to rethink the equation
    // between real point and texture coordinate...
    this->m_dLoadedBounds[0] =
        origin[0] + static_cast<double>(this->m_iExtents[0 + swapBounds[0]]) * spacing[0];
    this->m_dLoadedBounds[2] =
        origin[1] + static_cast<double>(this->m_iExtents[2 + swapBounds[1]]) * spacing[1];
    this->m_dLoadedBounds[4] =
        origin[2] + static_cast<double>(this->m_iExtents[4 + swapBounds[2]]) * spacing[2];
    this->m_dLoadedBounds[1] =
        origin[0] + static_cast<double>(this->m_iExtents[1 - swapBounds[0]]) * spacing[0];
    this->m_dLoadedBounds[3] =
        origin[1] + static_cast<double>(this->m_iExtents[3 - swapBounds[1]]) * spacing[1];
    this->m_dLoadedBounds[5] =
        origin[2] + static_cast<double>(this->m_iExtents[5 - swapBounds[2]]) * spacing[2];
    
    
    // Update sampling distance
    this->m_dDatasetStepSize[0] = 1.0 / (this->m_dLoadedBounds[1] - this->m_dLoadedBounds[0]);
    this->m_dDatasetStepSize[1] = 1.0 / (this->m_dLoadedBounds[3] - this->m_dLoadedBounds[2]);
    this->m_dDatasetStepSize[2] = 1.0 / (this->m_dLoadedBounds[5] - this->m_dLoadedBounds[4]);

    // Cell step/scale are adjusted per block.
    // Step should be dependent on the bounds and not on the texture size
    // since we can have a non-uniform voxel size / spacing / aspect ratio.
    this->m_fCellStep[0] = (1.f / static_cast<float>(this->m_iExtents[1] - this->m_iExtents[0]));
    this->m_fCellStep[1] = (1.f / static_cast<float>(this->m_iExtents[3] - this->m_iExtents[2]));
    this->m_fCellStep[2] = (1.f / static_cast<float>(this->m_iExtents[5] - this->m_iExtents[4]));

    this->m_fCellSpacing[0] = static_cast<float>(spacing[0]);
    this->m_fCellSpacing[1] = static_cast<float>(spacing[1]);
    this->m_fCellSpacing[2] = static_cast<float>(spacing[2]);
}

void SetVolumeParameter::CalculateParameter()
{
    int numVolumes = 1;
    this->m_vecVolMat.resize(numVolumes * 16, 0);
    this->m_vecInvMat.resize(numVolumes * 16, 0);
    this->m_vecTexMat.resize(numVolumes * 16, 0);
    this->m_vecInvTexMat.resize(numVolumes * 16, 0);
    this->m_vecTexEyeMat.resize(numVolumes * 16, 0);
    this->m_vecCellToPoint.resize(numVolumes * 16, 0);
    this->m_vecTexMin.resize(numVolumes * 3, 0);
    this->m_vecTexMax.resize(numVolumes * 3, 0);
    this->m_vecEyePos.resize(numVolumes * 3, 0);

    vtkNew<vtkMatrix4x4> dataToWorld;
    vtkNew<vtkMatrix4x4> dataToView;
    vtkNew<vtkMatrix4x4> texToDataMat;
    vtkNew<vtkMatrix4x4> texToViewMat;
    vtkNew<vtkMatrix4x4> cellToPointMat;

    float defaultTexMin[3] = { 0.0f, 0.0f, 0.0f };
    float defaultTexMax[3] = { 1.0f, 1.0f, 1.0f };
    float eyePos[3] = { 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < numVolumes; i++)
    {
        const int vecOfffset = i * 16;
        float* texMin;
        float* texMax;

        if (numVolumes > 1)
        {
        }
        else
        {
            vtkMatrix4x4* volMatrix = this->m_mat4TempMatrix4x4;
            // DeepCopy的参数是source 
            dataToWorld->DeepCopy(volMatrix);
            texToDataMat->DeepCopy(this->m_mat4TextureToDataset.GetPointer());

            // Texture matrices (texture to view)
            // Multiply4x4 => a * b = c
            vtkMatrix4x4::Multiply4x4(volMatrix, texToDataMat.GetPointer(), texToViewMat.GetPointer());
            //vtkMatrix4x4::Multiply4x4(modelViewMat, texToViewMat.GetPointer(), texToViewMat.GetPointer());
        }
    }
}

double* SetVolumeParameter::GetBound()
{
    int i, n;
    double bbox[24], * fptr;

    const double* bounds = m_spImageData->GetBounds();
    // Check for the special case when the mapper's bounds are unknown
    if (bounds == nullptr)
    {
        return nullptr;
    }

    // fill out vertices of a bounding box
    bbox[0] = bounds[1];
    bbox[1] = bounds[3];
    bbox[2] = bounds[5];
    bbox[3] = bounds[1];
    bbox[4] = bounds[2];
    bbox[5] = bounds[5];
    bbox[6] = bounds[0];
    bbox[7] = bounds[2];
    bbox[8] = bounds[5];
    bbox[9] = bounds[0];
    bbox[10] = bounds[3];
    bbox[11] = bounds[5];
    bbox[12] = bounds[1];
    bbox[13] = bounds[3];
    bbox[14] = bounds[4];
    bbox[15] = bounds[1];
    bbox[16] = bounds[2];
    bbox[17] = bounds[4];
    bbox[18] = bounds[0];
    bbox[19] = bounds[2];
    bbox[20] = bounds[4];
    bbox[21] = bounds[0];
    bbox[22] = bounds[3];
    bbox[23] = bounds[4];

    // and transform into actors coordinates
    fptr = bbox;
    for (n = 0; n < 8; n++)
    {
        double homogeneousPt[4] = { fptr[0], fptr[1], fptr[2], 1.0 };
        this->Matrix->MultiplyPoint(homogeneousPt, homogeneousPt);
        fptr[0] = homogeneousPt[0] / homogeneousPt[3];
        fptr[1] = homogeneousPt[1] / homogeneousPt[3];
        fptr[2] = homogeneousPt[2] / homogeneousPt[3];
        fptr += 3;
    }

    // now calc the new bounds
    this->m_dBounds[0] = this->m_dBounds[2] = this->m_dBounds[4] = VTK_DOUBLE_MAX;
    this->m_dBounds[1] = this->m_dBounds[3] = this->m_dBounds[5] = -VTK_DOUBLE_MAX;
    for (i = 0; i < 8; i++)
    {
        for (n = 0; n < 3; n++)
        {
            if (bbox[i * 3 + n] < this->m_dBounds[n * 2])
            {
                this->m_dBounds[n * 2] = bbox[i * 3 + n];
            }
            if (bbox[i * 3 + n] > this->m_dBounds[n * 2 + 1])
            {
                this->m_dBounds[n * 2 + 1] = bbox[i * 3 + n];
            }
        }
    }
    return this->m_dBounds;
}

void SetVolumeParameter::ComputeVisiblePropBounds(double allBounds[6])
{
    int nothingVisible = 1;
    allBounds[0] = allBounds[2] = allBounds[4] = VTK_DOUBLE_MAX;
    allBounds[1] = allBounds[3] = allBounds[5] = -VTK_DOUBLE_MAX;

    const double* bounds = this->GetBound();
    // make sure we haven't got bogus bounds
    if (bounds != nullptr && vtkMath::AreBoundsInitialized(bounds))
    {
        nothingVisible = 0;

        if (bounds[0] < allBounds[0])
        {
            allBounds[0] = bounds[0];
        }
        if (bounds[1] > allBounds[1])
        {
            allBounds[1] = bounds[1];
        }
        if (bounds[2] < allBounds[2])
        {
            allBounds[2] = bounds[2];
        }
        if (bounds[3] > allBounds[3])
        {
            allBounds[3] = bounds[3];
        }
        if (bounds[4] < allBounds[4])
        {
            allBounds[4] = bounds[4];
        }
        if (bounds[5] > allBounds[5])
        {
            allBounds[5] = bounds[5];
        }
    } // not bogus
}