#pragma once

#include <string>
#include <vector>
#include <optional>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

class Dicom2mesh
{
public:
    Dicom2mesh();
    ~Dicom2mesh() {};

    int DoMesh();
    vtkSmartPointer<vtkPolyData> DicomToMesh(const vtkSmartPointer<vtkImageData>& imageData, int threshold,
        bool useUpperThreshold = false, int upperThreshold = 0);

private:
    vtkSmartPointer<vtkCallbackCommand> m_spCallback;
};

