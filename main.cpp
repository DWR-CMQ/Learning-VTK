//#include <vtkActor.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkCellPicker.h>
//#include <vtkImagePlaneWidget.h>
//#include <vtkMarchingCubes.h>
//#include <vtkImageCast.h>
//#include <vtkVolumeRayCastCompositeFunction.h>
//#include <vtkVolumeProperty.h>
//#include <vtkVolume.h>
//#include <vtkVolumeRayCastMapper.h>
//#include <vtkPiecewiseFunction.h>
//#include <vtkColorTransferFunction.h>
//#include <vtkGPUVolumeRayCastMapper.h>
//#include <vtkImageMapToColors.h>
//#include <vtkProperty.h>
//#include <vtkImageActor.h>
//#include<vtkImageData.h>
//#include<vtkVolumeRayCastCompositeFunction.h>
//#include<vtkGPUVolumeRayCastMapper.h>
//#include<vtkSmartPointer.h>

#include "core/vtkSmartPointer.h"
#include "core/vtkDICOMImageReader.h"
#include "core/vtkImageData.h"
//#include "core/vtkSMPTools.h"

int main(int argc, char* argv[])
{

    std::string folder = "F:/Projects/BoneVisualization/201_240";

    //read all the dicom files with the received path.
    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(folder.c_str());
    reader->Update();

    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims); //need include <vtkimagedata.h>
    cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;



    return 0;
}