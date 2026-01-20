#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "va_shader.h"
class Render
{
public:
    Render();
    ~Render();
private:
    unsigned int m_uiVao;
    unsigned int m_uiVbo;

    Shader* m_pDrawShader = nullptr;
};

