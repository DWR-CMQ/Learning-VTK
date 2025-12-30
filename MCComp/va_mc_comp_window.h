#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <functional>
#include <glm/glm.hpp>

class MCWindow
{
public:
    MCWindow(int width, int height, const char* title, bool fullscreen = false);
    virtual ~MCWindow();
    GLFWwindow* GetWindow() const { return m_pWindow; }
    void setCameraUpdateCallback(std::function<void(float, float, float)> cb);
    glm::ivec2 GetWindowSize() const;
    glm::ivec2 GetFrameBufferSize() const;

private:
    static std::function<void(float, float, float)> cameraUpdateCallback;

    static void errorCallback(int error, const char* msg);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double new_cursor_x, double new_cursor_y);
    static void mouseScrollCallback(GLFWwindow* window, double x_offset, double y_offset);

    GLFWwindow* m_pWindow;
    int m_iWindowWidth;
    int m_iWindowHeight;
    int m_iFramebufferWidth;
    int m_iFramebufferHeight;

    bool mouse_button_pressed;
    double old_cursor_x, old_cursor_y;
};

