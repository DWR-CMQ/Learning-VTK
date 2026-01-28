#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
class VAWindow
{
public:
	VAWindow(int width, int height, const char* title, bool fullscreen = false);
	~VAWindow();
	void GetTiledSizeAndOrigin(int* usize, int* vsize, int* lowerLeftU, int* lowerLeftV);
	void NormalizedDisplayToDisplay(double& u, double& v);
	int* GetSize();
private:
	static std::function<void(float, float, float)> cameraUpdateCallback;

	static void errorCallback(int error, const char* msg);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double new_cursor_x, double new_cursor_y);
	static void mouseScrollCallback(GLFWwindow* window, double x_offset, double y_offset);

	int m_iWindowWidth;
	int m_iWindowHeight;
	int m_iFramebufferWidth;
	int m_iFramebufferHeight;

	GLFWwindow* m_pVAWindow;
	bool m_bMouseButtonPressed;
	double m_dOldCursorX, m_dOldCursorY;

	int Size[2];
	int TileSize[2];
	int TileScale[2];
};

