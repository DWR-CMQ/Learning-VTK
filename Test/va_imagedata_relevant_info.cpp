#include "va_imagedata_relevant_info.h"
#include <vtkMath.h>
ImageDataRelevantInfo::ImageDataRelevantInfo(vtkSmartPointer<vtkImageData> data)
{
    m_spImageData = data;
    Matrix = vtkMatrix4x4::New();
}

ImageDataRelevantInfo::~ImageDataRelevantInfo()
{
    // ¾ØÕóÎö¹¹
    this->Matrix->Delete();
    this->Matrix = nullptr;
}


void ImageDataRelevantInfo::Init()
{
    int i, n;
    double bbox[24], * fptr;

    const double* bounds = m_spImageData->GetBounds();
    // Check for the special case when the mapper's bounds are unknown
    if (bounds == nullptr)
    {
        return;
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
}

double* ImageDataRelevantInfo::GetBound()
{
    return m_dBounds;
}

void ImageDataRelevantInfo::ComputeVisiblePropBounds(double allBounds[6])
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
