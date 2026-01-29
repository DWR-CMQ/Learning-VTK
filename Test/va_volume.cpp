#include "va_volume.h"
#include <glad/glad.h>
#include <vtkAbstractMapper.h>
#include <vtkAbstractArray.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
VAVolume::VAVolume(const vtkSmartPointer<vtkImageData>& imageData)
{
	this->m_spImageData = imageData;
	this->m_spVolumeTexture = std::make_shared<TextureObject>(this->m_spImageData->GetScalarType());
    m_spVolumeProperty = std::make_shared<VAVolumeProperty>();

    ComputeBounds();
    Matrix = vtkMatrix4x4::New();

    this->CellToPointMatrix->Identity();
    this->AdjustedTexMin[0] = this->AdjustedTexMin[1] = this->AdjustedTexMin[2] = 0.0f;
    this->AdjustedTexMin[3] = 1.0f;
    this->AdjustedTexMax[0] = this->AdjustedTexMax[1] = this->AdjustedTexMax[2] = 1.0f;
    this->AdjustedTexMax[3] = 1.0f;

    this->ScalarRange[0][0] = this->ScalarRange[0][1] = 0.f;
    this->ScalarRange[1][0] = this->ScalarRange[1][1] = 0.f;
    this->ScalarRange[2][0] = this->ScalarRange[2][1] = 0.f;
    this->ScalarRange[3][0] = this->ScalarRange[3][1] = 0.f;

    this->Scale[0] = 1.0f;
    this->Scale[1] = 1.0f;
    this->Scale[2] = 1.0f;
    this->Scale[3] = 1.0f;

    this->Bias[0] = 0.0f;
    this->Bias[1] = 0.0f;
    this->Bias[2] = 0.0f;
    this->Bias[3] = 0.0f;
    this->HandleLargeDataTypes = false;
    this->InterpolationType = TextureObject::Linear;
}

VAVolume::~VAVolume()
{
    // 矩阵析构
    this->Matrix->Delete();
    this->Matrix = nullptr;
}

void VAVolume::LoadVolume()
{
    if (this->m_spImageData != nullptr)
    {
        this->m_spImageData->GetExtent(this->FullExtent.GetData());
    }

	if (this->m_spVolumeTexture == nullptr)
	{
		this->m_spVolumeTexture = std::make_shared<TextureObject>(this->m_spImageData->GetScalarType());
	}

	int scalarType = this->m_spImageData->GetScalarType();
	int noOfComponents = this->m_spImageData->GetNumberOfScalarComponents();
	unsigned int format = this->m_spVolumeTexture->GetDefaultFormat(scalarType, noOfComponents, false);
	unsigned int internalFormat = this->m_spVolumeTexture->GetDefaultInternalFormat(scalarType, noOfComponents, false);
	int type = this->m_spVolumeTexture->GetDefaultDataType(scalarType);
    this->SelectTextureFormat(format, internalFormat, type, scalarType, noOfComponents);

    this->CreateBlocks(format, internalFormat, type);
    this->LoadTexture(this->InterpolationType);
}

VAVolume::Size3 VAVolume::ComputeBlockSize(int* extent)
{
    int i = 0;
    Size3 texSize;
    while (i < 3)
    {
        texSize[i] = extent[2 * i + 1] - extent[2 * i] + 1;
        ++i;
    }
    return texSize;
}

// 这里使用VTK的同名函数,但是不引入VolumeBlock结构体
void VAVolume::CreateBlocks(unsigned int format, unsigned int internalFormat, int type)
{
    this->FullSize[0] = this->FullExtent[1] - this->FullExtent[0] + 1;
    this->FullSize[1] = this->FullExtent[3] - this->FullExtent[2] + 1;
    this->FullSize[2] = this->FullExtent[5] - this->FullExtent[4] + 1;

    int* ext = m_spImageData->GetExtent();
    TextureSize = this->ComputeBlockSize(ext);
    this->ComputeBounds();
    this->UpdateTextureToDataMatrix();

    this->ComputeCellToPointMatrix(this->FullExtent.GetData());

    // Format texture
    this->m_spVolumeTexture->SetFormat(format);
    this->m_spVolumeTexture->SetInternalFormat(internalFormat);
    this->m_spVolumeTexture->SetDataType(type);
}

void VAVolume::LoadTexture(int interpolation)
{
    int const noOfComponents = this->Scalars->GetNumberOfComponents();
    int scalarType = this->Scalars->GetDataType();
    int blockExt[6];
    this->m_spImageData->GetExtent(blockExt);
    if (!this->HandleLargeDataTypes)
    {
        void* dataPtr = m_spImageData->GetScalarPointer();
        this->m_spVolumeTexture->Create3DTextureFromRaw(TextureSize[0], TextureSize[1], TextureSize[2], noOfComponents, scalarType, dataPtr);
    }
    this->m_spVolumeTexture->ActivateTexture(0);
    this->m_spVolumeTexture->SetWrapSMode(TextureObject::ClampToEdge);
    this->m_spVolumeTexture->SetWrapTMode(TextureObject::ClampToEdge);
    this->m_spVolumeTexture->SetWrapRMode(TextureObject::ClampToEdge);
    this->m_spVolumeTexture->SetMinificationFilterMode(interpolation);
    this->m_spVolumeTexture->SetMagnificationFilterMode(interpolation);
    this->m_spVolumeTexture->DeActivateTexture();
}

void VAVolume::UpdateTextureToDataMatrix()
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

void VAVolume::ComputeBounds()
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

double* VAVolume::GetBound()
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

void VAVolume::ComputeVisiblePropBounds(double allBounds[6])
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

void VAVolume::ComputeCellToPointMatrix(int extents[6])
{
    this->CellToPointMatrix->Identity();
    this->AdjustedTexMin[0] = this->AdjustedTexMin[1] = this->AdjustedTexMin[2] = 0.0f;
    this->AdjustedTexMin[3] = 1.0f;
    this->AdjustedTexMax[0] = this->AdjustedTexMax[1] = this->AdjustedTexMax[2] = 1.0f;
    this->AdjustedTexMax[3] = 1.0f;

    if (!this->m_iIsCellData) // point data
    {
        // Extents are one minus the number of elements
        // so we have to add 1 to it to account for
        // number of elements in any cell or point image
        // data.
        float delta[3];
        delta[0] = extents[1] - extents[0] + 1;
        delta[1] = extents[3] - extents[2] + 1;
        delta[2] = extents[5] - extents[4] + 1;

        float min[3];
        min[0] = delta[0] > 0.0 ? 0.5f / delta[0] : 0.5f;
        min[1] = delta[1] > 0.0 ? 0.5f / delta[1] : 0.5f;
        min[2] = delta[2] > 0.0 ? 0.5f / delta[2] : 0.5f;

        float range[3]; // max - min
        range[0] = (delta[0] - 0.5f) / delta[0] - min[0];
        range[1] = (delta[1] - 0.5f) / delta[1] - min[1];
        range[2] = (delta[2] - 0.5f) / delta[2] - min[2];

        this->CellToPointMatrix->SetElement(0, 0, range[0]); // Scale diag
        this->CellToPointMatrix->SetElement(1, 1, range[1]);
        this->CellToPointMatrix->SetElement(2, 2, range[2]);
        this->CellToPointMatrix->SetElement(0, 3, min[0]); // t vector
        this->CellToPointMatrix->SetElement(1, 3, min[1]);
        this->CellToPointMatrix->SetElement(2, 3, min[2]);

        // Adjust limit coordinates for texture access.
        float const zeros[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // GL tex min
        float const ones[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // GL tex max
        this->CellToPointMatrix->MultiplyPoint(zeros, this->AdjustedTexMin);
        this->CellToPointMatrix->MultiplyPoint(ones, this->AdjustedTexMax);
    }
}

void VAVolume::GetScaleAndBias(int scalarType, float* scalarRange, float& scale, float& bias)
{
    scale = 1.0f;
    bias = 0.0f;
    double glScale = 1.0;
    double glBias = 0.0;

    switch (scalarType)
    {
    case VTK_UNSIGNED_CHAR:
        glScale = 1.0 / (VTK_UNSIGNED_CHAR_MAX + 1);
        glBias = 0.0;
        break;
    case VTK_SIGNED_CHAR:
        glScale = 2.0 / (VTK_UNSIGNED_CHAR_MAX + 1);
        glBias = -1.0 - VTK_SIGNED_CHAR_MIN * glScale;
        break;
    case VTK_SHORT:
        glScale = 2.0 / (VTK_UNSIGNED_SHORT_MAX + 1);
        glBias = -1.0 - VTK_SHORT_MIN * glScale;
        break;
    case VTK_UNSIGNED_SHORT:
        glScale = 1.0 / (VTK_UNSIGNED_SHORT_MAX + 1);
        glBias = 0.0;
        break;
    case VTK_CHAR:
    case VTK_BIT:
    case VTK_ID_TYPE:
    case VTK_STRING:
        // not supported
        assert("check: impossible case" && 0);
        break;
    }

    double glRange[2];
    for (int i = 0; i < 2; ++i)
    {
        glRange[i] = scalarRange[i] * glScale + glBias;
    }
    glRange[1] = (glRange[1] == glRange[0] ? glRange[0] + 1e-6 : glRange[1]);
    scale = static_cast<float>(1.0 / (glRange[1] - glRange[0]));
    bias = static_cast<float>(0.0 - glRange[0] * scale);
}

void VAVolume::SelectTextureFormat(unsigned int& format, unsigned int& internalFormat, int& type, int scalarType, int noOfComponents)
{
    bool supportsFloat = true;
    this->HandleLargeDataTypes = false;

    switch (scalarType)
    {
    case VTK_FLOAT:
        if (supportsFloat)
        {
            switch (noOfComponents)
            {
            case 1:
                internalFormat = GL_R32F;
                format = GL_RED;
                break;
            case 2:
                internalFormat = GL_RG32F;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB32F;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA32F;
                format = GL_RGBA;
                break;
            default:
                break;
            }
        }
        else
        {
            switch (noOfComponents)
            {
            case 1:
                internalFormat = GL_RED;
                format = GL_RED;
                break;
            case 2:
                internalFormat = GL_RG;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA;
                format = GL_RGBA;
                break;
            default:
                break;
            }
        }

    case VTK_UNSIGNED_CHAR:
    case VTK_SIGNED_CHAR:
    case VTK_SHORT:
    case VTK_UNSIGNED_SHORT:
        // Nothing to be done
        break;
    case VTK_INT:
    case VTK_DOUBLE:
    case VTK_LONG:
    case VTK_LONG_LONG:
    case VTK_UNSIGNED_INT:
    case VTK_UNSIGNED_LONG:
    case VTK_UNSIGNED_LONG_LONG:
        this->HandleLargeDataTypes = true;
        type = GL_FLOAT;
        switch (noOfComponents)
        {
        case 1:
            if (supportsFloat)
            {
                internalFormat = GL_R32F;
            }
            else
            {
                internalFormat = GL_RED;
            }
            format = GL_RED;
            break;
        case 2:
            internalFormat = GL_RG;
            format = GL_RG;
            break;
        case 3:
            internalFormat = GL_RGB;
            format = GL_RGB;
            break;
        case 4:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            break;
        }
        break;
    case VTK_CHAR:
    case VTK_BIT:
    case VTK_ID_TYPE:
    case VTK_STRING:
    default:
        std::cout << "check: impossible case" << std::endl;
        break;
    }

    int ScalarMode = 0;
    char* ArrayName = new char[1];
    ArrayName[0] = '\0';
    int ArrayId = -1;
    int ArrayAccessMode = VTK_GET_ARRAY_BY_ID;
    int cellFlag = 0;
    vtkAbstractArray* abstractScalars = vtkAbstractMapper::GetAbstractScalars(m_spImageData, ScalarMode, ArrayAccessMode, ArrayId, ArrayName, cellFlag);
    this->Scalars = vtkArrayDownCast<vtkDataArray>(abstractScalars);

    for (int n = 0; n < noOfComponents; n++)
    {
        double* range = this->Scalars->GetFiniteRange(n);
        for (int i = 0; i < 2; ++i)
        {
            this->ScalarRange[n][i] = static_cast<float>(range[i]);
        }
    }

    // Pixel Transfer NI to LUT Tex.Coord. [0, 1]
    // NP = P * scale + bias
    // Given two point matches a,b to c,d the formulas are:
    // scale = (d - c) / (b - a)
    // bias = c - a * scale
    // For unsigned/float types c is zero.
    int const components = vtkMath::Min(noOfComponents, 4);
    for (int n = 0; n < components; n++)
    {
        this->GetScaleAndBias(scalarType, this->ScalarRange[n], this->Scale[n], this->Bias[n]);
    }
}

std::shared_ptr<VAVolumeProperty> VAVolume::GetVolumeProperty()
{
    return m_spVolumeProperty;
}

vtkDataArray* VAVolume::GetLoadedScalars()
{
    return this->Scalars;
}