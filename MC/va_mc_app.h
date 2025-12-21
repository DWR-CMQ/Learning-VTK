#pragma once
#include "va_mc_camera.h"
#include "vtkDICOMImageReader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "va_mc_volume.h"
#include "va_mc_render.h"
#include "va_mc_window.h"

class MCApp
{
public:
    void Init(bool bFullScreen = true);
    void Run();
    void Restart();
    void Close();

public:
    MCApp();
    ~MCApp();

private:
    std::shared_ptr<MCRender> m_spRender;
    std::shared_ptr<MCWindow> m_spWindow;
    float lastX = 800.f / 2.0;
    float lastY = 600.f / 2.0;
    bool firstMouse = true;
};

