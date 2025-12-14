#pragma once
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageAlgorithm.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
class VolumeVisualizer
{
public:
    VolumeVisualizer();
    virtual ~VolumeVisualizer();
    void DisplayVolume(vtkSmartPointer<vtkRenderWindow> renderWindow,
                                vtkAlgorithmOutput* volumeData);
    vtkSmartPointer<vtkRenderer> GetRenderer();
private:
    vtkSmartPointer<vtkRenderer> m_spVolumeRenderer;
};

