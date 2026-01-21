#include "va_camera.h"

Camera::Camera(vtkSmartPointer<vtkCamera> camera, double* bounds)
{
	m_spCamera = camera;
    this->ClippingRangeExpansion = 0.5;
    this->NearClippingPlaneTolerance = 0;
    memcpy(m_dBounds, bounds, 6 * sizeof(double));
    this->WCDCMatrix = vtkMatrix4x4::New();
    this->WCVCMatrix = vtkMatrix4x4::New();
    this->NormalMatrix = vtkMatrix3x3::New();
    this->VCDCMatrix = vtkMatrix4x4::New();
}

Camera::~Camera()
{
    this->WCDCMatrix->Delete();
    this->WCVCMatrix->Delete();
    this->NormalMatrix->Delete();
    this->VCDCMatrix->Delete();
}

void Camera::ResetCameraClippingRange(const double bounds[6])
{
    double vn[3], position[3], a, b, c, d;
    double range[2], dist;
    int i, j, k;

    // Don't reset the clipping range when we don't have any 3D visible props
    if (!vtkMath::AreBoundsInitialized(bounds))
    {
        return;
    }

    if (this->m_spCamera == nullptr)
    {
        std::cout << "Trying to reset clipping range of non-existent camera" << std::endl;
        return;
    }

    double expandedBounds[6] = { bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
    if (!this->m_spCamera->GetUseOffAxisProjection())
    {
        this->m_spCamera->GetViewPlaneNormal(vn);
        this->m_spCamera->GetPosition(position);
    }
    else
    {
        this->m_spCamera->GetEyePosition(position);
        this->m_spCamera->GetEyePlaneNormal(vn);
    }

    this->ExpandBounds(expandedBounds, this->m_spCamera->GetModelTransformMatrix());

    a = -vn[0];
    b = -vn[1];
    c = -vn[2];
    d = -(a * position[0] + b * position[1] + c * position[2]);

    // Set the max near clipping plane and the min far clipping plane
    range[0] = a * expandedBounds[0] + b * expandedBounds[2] + c * expandedBounds[4] + d;
    range[1] = 1e-18;

    // Find the closest / farthest bounding box vertex
    for (k = 0; k < 2; k++)
    {
        for (j = 0; j < 2; j++)
        {
            for (i = 0; i < 2; i++)
            {
                dist = a * expandedBounds[i] + b * expandedBounds[2 + j] + c * expandedBounds[4 + k] + d;
                range[0] = (dist < range[0]) ? (dist) : (range[0]);
                range[1] = (dist > range[1]) ? (dist) : (range[1]);
            }
        }
    }

    // do not let far - near be less than 0.1 of the window height
    // this is for cases such as 2D images which may have zero range
    double minGap = 0.0;
    if (this->m_spCamera->GetParallelProjection())
    {
        minGap = 0.1 * this->m_spCamera->GetParallelScale();
    }
    else if (this->m_spCamera->GetUseOffAxisProjection())
    {
        double offAxisAdustment = this->m_spCamera->GetOffAxisClippingAdjustment();
        range[0] -= offAxisAdustment;
        range[1] += offAxisAdustment;
    }
    else
    {
        double angle = vtkMath::RadiansFromDegrees(this->m_spCamera->GetViewAngle());
        minGap = 0.2 * tan(angle / 2.0) * range[1];
    }
    if (range[1] - range[0] < minGap)
    {
        minGap = minGap - range[1] + range[0];
        range[1] += minGap / 2.0;
        range[0] -= minGap / 2.0;
    }

    // Do not let the range behind the camera throw off the calculation.
    if (range[0] < 0.0)
    {
        range[0] = 0.0;
    }

    // Give ourselves a little breathing room
    range[0] = 0.99 * range[0] - (range[1] - range[0]) * this->ClippingRangeExpansion;
    range[1] = 1.01 * range[1] + (range[1] - range[0]) * this->ClippingRangeExpansion;

    // Make sure near is not bigger than far
    range[0] = (range[0] >= range[1]) ? (0.01 * range[1]) : (range[0]);

    // Make sure near is at least some fraction of far - this prevents near
    // from being behind the camera or too close in front. How close is too
    // close depends on the resolution of the depth buffer
    if (!this->NearClippingPlaneTolerance)
    {
        this->NearClippingPlaneTolerance = 0.001;
    }

    // make sure the front clipping range is not too far from the far clippnig
    // range, this is to make sure that the zbuffer resolution is effectively
    // used
    if (range[0] < this->NearClippingPlaneTolerance * range[1])
    {
        range[0] = this->NearClippingPlaneTolerance * range[1];
    }

    this->m_spCamera->SetClippingRange(range);
}

void Camera::Init()
{
    double center[3];
    double distance;
    double vn[3], * vup;

    if (this->m_spCamera != nullptr)
    {
        this->m_spCamera->GetViewPlaneNormal(vn);
    }
    else
    {
        std::cout << "Trying to reset non-existent camera" << std::endl;
        return;
    }

    // Reset the perspective zoom factors, otherwise subsequent zooms will cause
    // the view angle to become very small and cause bad depth sorting.
    this->m_spCamera->SetViewAngle(30.0);

    double expandedBounds[6] = { m_dBounds[0], 
                                m_dBounds[1], 
                                m_dBounds[2], 
                                m_dBounds[3], 
                                m_dBounds[4], 
                                m_dBounds[5] };
    this->ExpandBounds(expandedBounds, this->m_spCamera->GetModelTransformMatrix());

    center[0] = (expandedBounds[0] + expandedBounds[1]) / 2.0;
    center[1] = (expandedBounds[2] + expandedBounds[3]) / 2.0;
    center[2] = (expandedBounds[4] + expandedBounds[5]) / 2.0;

    double w1 = expandedBounds[1] - expandedBounds[0];
    double w2 = expandedBounds[3] - expandedBounds[2];
    double w3 = expandedBounds[5] - expandedBounds[4];
    w1 *= w1;
    w2 *= w2;
    w3 *= w3;
    double radius = w1 + w2 + w3;

    // If we have just a single point, pick a radius of 1.0
    radius = (radius == 0) ? (1.0) : (radius);

    // compute the radius of the enclosing sphere
    radius = sqrt(radius) * 0.5;

    double angle = vtkMath::RadiansFromDegrees(this->m_spCamera->GetViewAngle());
    double parallelScale = radius;
    double aspect[2] = { 1.0,1.0 };

    if (aspect[0] >= 1.0) // horizontal window, deal with vertical angle|scale
    {
        if (this->m_spCamera->GetUseHorizontalViewAngle())
        {
            angle = 2.0 * atan(tan(angle * 0.5) / aspect[0]);
        }
    }
    else // vertical window, deal with horizontal angle|scale
    {
        if (!this->m_spCamera->GetUseHorizontalViewAngle())
        {
            angle = 2.0 * atan(tan(angle * 0.5) * aspect[0]);
        }

        parallelScale = parallelScale / aspect[0];
    }

    distance = radius / sin(angle * 0.5);

    // check view-up vector against view plane normal
    vup = this->m_spCamera->GetViewUp();
    if (fabs(vtkMath::Dot(vup, vn)) > 0.999)
    {
        std::cout << "Resetting view-up since view plane normal is parallel" << std::endl;
        this->m_spCamera->SetViewUp(-vup[2], vup[0], vup[1]);
    }

    // update the camera
    this->m_spCamera->SetFocalPoint(center[0], center[1], center[2]);
    this->m_spCamera->SetPosition(
        center[0] + distance * vn[0], center[1] + distance * vn[1], center[2] + distance * vn[2]);

    this->ResetCameraClippingRange(expandedBounds);

    // setup default parallel scale
    this->m_spCamera->SetParallelScale(parallelScale);

}

void Camera::ExpandBounds(double bounds[6], vtkMatrix4x4* matrix)
{
    if (bounds == NULL)
    {
        std::cout << "ERROR: Invalid bounds" << std::endl;
        return;
    }

    if (matrix == NULL)
    {
        std::cout << "ERROR: Invalid matrix" << std::endl;
        return;
    }

    // Expand the bounding box by model view transform matrix.
    double pt[8][4] = { { bounds[0], bounds[2], bounds[5], 1.0 },
      { bounds[1], bounds[2], bounds[5], 1.0 }, { bounds[1], bounds[2], bounds[4], 1.0 },
      { bounds[0], bounds[2], bounds[4], 1.0 }, { bounds[0], bounds[3], bounds[5], 1.0 },
      { bounds[1], bounds[3], bounds[5], 1.0 }, { bounds[1], bounds[3], bounds[4], 1.0 },
      { bounds[0], bounds[3], bounds[4], 1.0 } };

    // \note: Assuming that matrix doesn not have projective component. Hence not
    // dividing by the homogeneous coordinate after multiplication
    for (int i = 0; i < 8; ++i)
    {
        matrix->MultiplyPoint(pt[i], pt[i]);
    }

    // min = mpx = pt[0]
    double min[4], max[4];
    for (int i = 0; i < 4; ++i)
    {
        min[i] = pt[0][i];
        max[i] = pt[0][i];
    }

    for (int i = 1; i < 8; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (min[j] > pt[i][j])
                min[j] = pt[i][j];
            if (max[j] < pt[i][j])
                max[j] = pt[i][j];
        }
    }

    // Copy values back to bounds.
    bounds[0] = min[0];
    bounds[2] = min[1];
    bounds[4] = min[2];

    bounds[1] = max[0];
    bounds[3] = max[1];
    bounds[5] = max[2];
}

void Camera::GetKeyMatrices(vtkMatrix4x4*& wcvc, vtkMatrix3x3*& normMat, vtkMatrix4x4*& vcdc, vtkMatrix4x4*& wcdc)
{
    this->WCVCMatrix->DeepCopy(this->m_spCamera->GetModelViewTransformMatrix());

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            this->NormalMatrix->SetElement(i, j, this->WCVCMatrix->GetElement(i, j));
        }
    }
    this->NormalMatrix->Invert();

    this->WCVCMatrix->Transpose();

    this->VCDCMatrix->DeepCopy(this->m_spCamera->GetProjectionTransformMatrix(1.0, -1, 1));
    this->VCDCMatrix->Transpose();

    vtkMatrix4x4::Multiply4x4(this->WCVCMatrix, this->VCDCMatrix, this->WCDCMatrix);
    wcvc = this->WCVCMatrix;
    normMat = this->NormalMatrix;
    vcdc = this->VCDCMatrix;
    wcdc = this->WCDCMatrix;
}