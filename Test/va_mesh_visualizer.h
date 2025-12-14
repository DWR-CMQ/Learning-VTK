#pragma once

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <chrono>

class MeshVisualizer
{
public:
	MeshVisualizer();
	virtual ~MeshVisualizer();
	void DisplayMesh(vtkSmartPointer<vtkRenderWindow> renderWindow,
							const vtkSmartPointer<vtkPolyData>& spMesh);

	vtkSmartPointer<vtkRenderer> GetRenderer();
private:
	vtkSmartPointer<vtkRenderer> m_spVolumeRenderer;
};

class MeshVisualizerInteraction : public vtkInteractorStyleTrackballCamera
{
public:
	static MeshVisualizerInteraction* New();
	vtkTypeMacro(MeshVisualizerInteraction, vtkInteractorStyleTrackballCamera);
	virtual void OnLeftButtonDown() override;
		
private:
	MeshVisualizerInteraction();
	void InitIfNecessary();
	bool IsDoubleClick();

private:
	vtkSmartPointer<vtkActor> m_spActor;
	std::chrono::system_clock::time_point m_lastMouseClick;
};

