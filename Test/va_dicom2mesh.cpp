#include "va_dicom2mesh.h"
#include <vtkDICOMImageReader.h>
#include <vtkImageThreshold.h>
#include <vtkMarchingCubes.h>
#include <windows.h>

void Volume2MeshProgressCallback(vtkObject* caller, long unsigned int /*eventId*/, void* /*clientData*/, void* /*callData*/)
{
	// display progress in terminal
	vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
	std::cout << "\33[2K\r"; // erase line
	std::cout << "Progress: ";
	if (filter->GetProgress() > 0.999)
		std::cout << "done";
	else
		std::cout << std::fixed << std::setprecision(1) << filter->GetProgress() * 100 << "%";
	std::cout << std::flush;
}

Dicom2mesh::Dicom2mesh()
{
	m_spCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	m_spCallback->SetCallback(Volume2MeshProgressCallback);
}

vtkSmartPointer<vtkPolyData> Dicom2mesh::DicomToMesh(const vtkSmartPointer<vtkImageData>& imageData, int threshold,
														bool useUpperThreshold , int upperThreshold)
{
	auto spCopyData = vtkSmartPointer<vtkImageData>::New();
	spCopyData->DeepCopy(imageData);
	if (useUpperThreshold)
	{
		cout << "Create surface mesh with iso value range = " << threshold << " to " << upperThreshold << endl;

		vtkSmartPointer<vtkImageThreshold> imageThreshold = vtkSmartPointer<vtkImageThreshold>::New();
		imageThreshold->SetInputData(spCopyData);
		imageThreshold->ThresholdByUpper(upperThreshold);
		imageThreshold->ReplaceInOn();
		imageThreshold->SetInValue(threshold - 1); // mask voxels with a value lower than the lower threshold
		imageThreshold->Update();
		spCopyData->DeepCopy(imageThreshold->GetOutput());
	}
	else
	{
		cout << "Create surface mesh with iso value = " << threshold << endl;
	}

	auto spMC = vtkSmartPointer<vtkMarchingCubes>::New();
	spMC->ComputeNormalsOn();
	spMC->SetValue(0, threshold);
	spMC->SetInputData(spCopyData);
	spMC->Update();

	std::cout << "生成的网格顶点数: " << spMC->GetOutput()->GetNumberOfPoints() << std::endl;
	std::cout << "生成的网格面片数: " << spMC->GetOutput()->GetNumberOfPolys() << std::endl;

	auto spMesh = vtkSmartPointer<vtkPolyData>::New();
	spMesh->DeepCopy(spMC->GetOutput());

	cout << endl << endl;
	return spMesh;
}


int Dicom2mesh::DoMesh()
{
	return 0;
}