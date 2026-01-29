#include "va_window.h"
#include <vtkMath.h>
#include <glad/glad.h>
std::function<void(float, float, float)> VAWindow::cameraUpdateCallback;
VAWindow::VAWindow(int width, int height, const char* title, bool fullscreen)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // nullptr indicates windowed.
    GLFWmonitor* monitor = nullptr;
    if (fullscreen)
    {
        monitor = glfwGetPrimaryMonitor();
    }

    m_bMouseButtonPressed = false;
    m_iWindowWidth = width;
    m_iWindowHeight = height;
    m_pVAWindow = glfwCreateWindow(width, height, "MC", monitor, nullptr);
    glfwMakeContextCurrent(m_pVAWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(m_pVAWindow, this);
    glfwSetInputMode(m_pVAWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetMouseButtonCallback(m_pVAWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(m_pVAWindow, cursorPosCallback);
    glfwSetScrollCallback(m_pVAWindow, mouseScrollCallback);
    glfwGetFramebufferSize(m_pVAWindow, &m_iFramebufferWidth, &m_iFramebufferHeight);
    glViewport(0, 0, m_iFramebufferWidth, m_iFramebufferHeight);

    this->Size[0] = width;
    this->Size[1] = height;
    this->TileSize[0] = 0;
    this->TileSize[1] = 0;
    this->TileScale[0] = 1;
    this->TileScale[1] = 1;
}

VAWindow::~VAWindow()
{
}

int VAWindow::GetWindowWidth()
{
    return this->m_iWindowWidth;
}

int VAWindow::GetWindowHeight()
{
    return this->m_iWindowHeight;
}

int* VAWindow::GetSize()
{
    this->TileSize[0] = this->Size[0] * this->TileScale[0];
    this->TileSize[1] = this->Size[1] * this->TileScale[1];

    return this->TileSize;
}

void VAWindow::NormalizedDisplayToDisplay(double& u, double& v)
{
    u = u * this->Size[0];
    v = v * this->Size[1];
}

void VAWindow::GetTiledSizeAndOrigin(int* usize, int* vsize, int* lowerLeftU, int* lowerLeftV)
{
    double* vport = new double[4];
    vport[0] = 0.0;      
    vport[1] = 0.0;      
    vport[2] = 0.5;  
    vport[3] = 1.0;  

    // if there is no window assume 0 1
    double tileViewPort[4];

    tileViewPort[0] = 0;
    tileViewPort[1] = 0;
    tileViewPort[2] = 1;
    tileViewPort[3] = 1;
    
    // find the lower left corner of the viewport, taking into account the
    // lower left boundary of this tile
    double vpu = vtkMath::ClampValue(vport[0] - tileViewPort[0], 0.0, 1.0);
    double vpv = vtkMath::ClampValue(vport[1] - tileViewPort[1], 0.0, 1.0);
    // store the result as a pixel value
    this->NormalizedDisplayToDisplay(vpu, vpv);
    *lowerLeftU = static_cast<int>(vpu + 0.5);
    *lowerLeftV = static_cast<int>(vpv + 0.5);

    // find the upper right corner of the viewport, taking into account the
    // lower left boundary of this tile
    double vpu2 = vtkMath::ClampValue(vport[2] - tileViewPort[0], 0.0, 1.0);
    double vpv2 = vtkMath::ClampValue(vport[3] - tileViewPort[1], 0.0, 1.0);
    // also watch for the upper right boundary of the tile
    if (vpu2 > (tileViewPort[2] - tileViewPort[0]))
    {
        vpu2 = tileViewPort[2] - tileViewPort[0];
    }
    if (vpv2 > (tileViewPort[3] - tileViewPort[1]))
    {
        vpv2 = tileViewPort[3] - tileViewPort[1];
    }
    this->NormalizedDisplayToDisplay(vpu2, vpv2);
    // now compute the size of the intersection of the viewport with the
    // current tile
    *usize = static_cast<int>(vpu2 + 0.5) - *lowerLeftU;
    *vsize = static_cast<int>(vpv2 + 0.5) - *lowerLeftV;
    if (*usize < 0)
    {
        *usize = 0;
    }
    if (*vsize < 0)
    {
        *vsize = 0;
    }

    delete[] vport;
}

void VAWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}
void VAWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    VAWindow* ptr = (VAWindow*)glfwGetWindowUserPointer(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        ptr->m_bMouseButtonPressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        ptr->m_bMouseButtonPressed = false;
    }
}

void VAWindow::cursorPosCallback(GLFWwindow* window, double new_cursor_x, double new_cursor_y)
{
    VAWindow* ptr = (VAWindow*)glfwGetWindowUserPointer(window);
    if (ptr->m_bMouseButtonPressed)
    {
        float delta_x, delta_y;
        float zenith = 0.06, azimuth = 0.06;

        delta_x = new_cursor_x - ptr->m_dOldCursorX;
        delta_y = new_cursor_y - ptr->m_dOldCursorY;

        if (delta_x < 1 && delta_x > -1)
            azimuth = 0;
        else if (delta_x > 0)
            azimuth *= -1.0;

        if (delta_y < 1 && delta_y > -1)
            zenith = 0;
        else if (delta_y > 0)
            zenith *= -1.0;

        VAWindow::cameraUpdateCallback(0, zenith, azimuth);

        ptr->m_dOldCursorX = new_cursor_x;
        ptr->m_dOldCursorY = new_cursor_y;
    }
    else
    {
        ptr->m_dOldCursorX = new_cursor_x;
        ptr->m_dOldCursorY = new_cursor_y;
    }
}

void VAWindow::mouseScrollCallback(GLFWwindow* window, double x_offset, double y_offset)
{
    cameraUpdateCallback(y_offset, 0, 0);
}