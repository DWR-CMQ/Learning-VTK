#include "va_mesh_visualizer.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropPicker.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkCamera.h>

MeshVisualizer::MeshVisualizer()
{
    m_spVolumeRenderer = vtkSmartPointer<vtkRenderer>::New();
}

MeshVisualizer::~MeshVisualizer()
{

}

vtkSmartPointer<vtkRenderer> MeshVisualizer::GetRenderer()
{
    return m_spVolumeRenderer;
}

void MeshVisualizer::DisplayMesh(vtkSmartPointer<vtkRenderWindow> renderWindow,
                                    const vtkSmartPointer<vtkPolyData>& spMesh)
{
    m_spVolumeRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_spVolumeRenderer->SetViewport(0.5, 0.0, 1.0, 1.0);
    renderWindow->AddRenderer(m_spVolumeRenderer);

    //auto meshInteraction = vtkSmartPointer<MeshVisualizerInteraction>::New();
    //meshInteraction->SetDefaultRenderer(renderer);

    //auto renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    //renderWindowInteractor->SetInteractorStyle(meshInteraction);
    //renderWindowInteractor->SetRenderWindow(renderWindow);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarVisibility(0);
    mapper->SetInputData(spMesh);

    auto spActor = vtkSmartPointer<vtkActor>::New();
    spActor->SetMapper(mapper);
    spActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    m_spVolumeRenderer->AddActor(spActor);
}

MeshVisualizerInteraction* MeshVisualizerInteraction::New()
{
    MeshVisualizerInteraction* result = new MeshVisualizerInteraction();
    result->InitializeObjectBase();
    return result;
}

MeshVisualizerInteraction::MeshVisualizerInteraction()
{
}

void MeshVisualizerInteraction::InitIfNecessary()
{
    if (m_spActor == nullptr)
    {
        auto spSphereSource = vtkSmartPointer<vtkSphereSource>::New();
        spSphereSource->SetRadius(0.4);

        auto spMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        spMapper->SetInputConnection(spSphereSource->GetOutputPort());

        m_spActor = vtkSmartPointer<vtkActor>::New();
        m_spActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
        m_spActor->SetMapper(spMapper);

        this->GetDefaultRenderer()->AddActor(m_spActor);
        m_lastMouseClick = std::chrono::system_clock::now();
    }
}

void MeshVisualizerInteraction::OnLeftButtonDown()
{
    InitIfNecessary();

    if (IsDoubleClick())
    {
        // mouse click position
        int* clickPos = this->GetInteractor()->GetEventPosition();

        // pick surface in 3d scene
        vtkSmartPointer<vtkPropPicker>  picker = vtkSmartPointer<vtkPropPicker>::New();
        picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());

        double* pos = picker->GetPickPosition();
        std::cout << "Pick position: [ " << std::fixed << std::setprecision(3) << pos[0] << "; " << pos[1] << "; " << pos[2] << "; 1.0 ] " << std::endl;

        m_spActor->SetPosition(pos);
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

bool MeshVisualizerInteraction::IsDoubleClick()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration<double> timeBetweenClicks = now - m_lastMouseClick;
    m_lastMouseClick = now;
    return timeBetweenClicks.count() < 0.5; // 0.5 seconds
}