#include "va_mc_window.h"
#include <iostream>

std::function<void(float, float, float)> MCWindow::cameraUpdateCallback;

MCWindow::MCWindow(int width, int height, const char* title, bool fullscreen)
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

    mouse_button_pressed = false;
    m_iWindowWidth = width;
    m_iWindowHeight = height;
    m_pWindow = glfwCreateWindow(width, height, "MC", monitor, nullptr);
    glfwMakeContextCurrent(m_pWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetMouseButtonCallback(m_pWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(m_pWindow, cursorPosCallback);
    glfwSetScrollCallback(m_pWindow, mouseScrollCallback);
    //glfwSetFramebufferSizeCallback(m_pWindow, framebufferSizeCallback);
    glfwGetFramebufferSize(m_pWindow, &m_iFramebufferWidth, &m_iFramebufferHeight);
    glViewport(0, 0, m_iFramebufferWidth, m_iFramebufferHeight);

    //glfwIconifyWindow(m_pWindow);
    //glfwSwapInterval(0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glfwSwapBuffers(m_pWindow);
    //glfwPollEvents();
}

MCWindow::~MCWindow()
{

}

glm::ivec2 MCWindow::GetWindowSize() const
{
    return glm::ivec2(m_iWindowWidth, m_iWindowHeight);
}

glm::ivec2 MCWindow::GetFrameBufferSize() const
{
    return glm::ivec2(m_iFramebufferWidth, m_iFramebufferHeight);
}

void MCWindow::setCameraUpdateCallback(std::function<void(float, float, float)> cb)
{
    MCWindow::cameraUpdateCallback = cb;
}

void MCWindow::errorCallback(int error, const char* msg)
{
    std::cout << msg << std::endl;
}

void MCWindow::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{

}

void MCWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}
void MCWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    MCWindow* ptr = (MCWindow*)glfwGetWindowUserPointer(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        ptr->mouse_button_pressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        ptr->mouse_button_pressed = false;
    }
}

void MCWindow::cursorPosCallback(GLFWwindow* window, double new_cursor_x, double new_cursor_y)
{
    MCWindow* ptr = (MCWindow*)glfwGetWindowUserPointer(window);
    if (ptr->mouse_button_pressed)
    {
        float delta_x, delta_y;
        float zenith = 0.06, azimuth = 0.06;

        delta_x = new_cursor_x - ptr->old_cursor_x;
        delta_y = new_cursor_y - ptr->old_cursor_y;

        if (delta_x < 1 && delta_x > -1)
            azimuth = 0;
        else if (delta_x > 0)
            azimuth *= -1.0;

        if (delta_y < 1 && delta_y > -1)
            zenith = 0;
        else if (delta_y > 0)
            zenith *= -1.0;

        MCWindow::cameraUpdateCallback(0, zenith, azimuth);

        ptr->old_cursor_x = new_cursor_x;
        ptr->old_cursor_y = new_cursor_y;
    }
    else
    {
        ptr->old_cursor_x = new_cursor_x;
        ptr->old_cursor_y = new_cursor_y;
    }
}

void MCWindow::mouseScrollCallback(GLFWwindow* window, double x_offset, double y_offset)
{
    cameraUpdateCallback(y_offset, 0, 0);
}