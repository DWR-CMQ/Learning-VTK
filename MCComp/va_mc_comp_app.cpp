#include "va_mc_comp_app.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vtkSmartPointer.h>
#include "va_mc_comp_common_function.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

MCApp::MCApp()
{
    m_spWindow = std::make_shared<MCWindow>(800, 600, "MC", false);
}

MCApp::~MCApp()
{
}

void MCApp::Init(bool bFullScreen)
{
    std::string dicomDirectory = "F:/DicomDataSet/Circle of Willis";
    if (!CommonFunction::CheckDICOMDirectory(dicomDirectory))
    {
        std::cout << "CheckDICOMDirectory Failed" << std::endl;
        return;
    }

    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(dicomDirectory.c_str());
    reader->Update();
    vtkImageData* imageData = reader->GetOutput();

    // 相关变量
    m_spRender = std::make_shared<MCRender>(imageData, m_spWindow->GetWindowSize(), m_spWindow->GetFrameBufferSize());
    m_spRender->SetUp();
    m_spWindow->setCameraUpdateCallback(std::bind(&(MCCamera::setOrientation), m_spRender->m_spCamera,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));

    m_spRender->LoadShader();
}

void MCApp::Run()
{
    while (!glfwWindowShouldClose(m_spWindow->GetWindow()))
    {
        glClearColor(0.0f, 0.5f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        m_spRender->Render();
        glfwSwapBuffers(m_spWindow->GetWindow());
        glfwPollEvents();
    }
}

void MCApp::Restart()
{
}

void MCApp::Close()
{
}
