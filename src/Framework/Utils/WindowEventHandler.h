#pragma once

#include "GLFW/glfw3.h"
class Camera;

class WindowEventHandler {

public:
	static void BindWindow(GLFWwindow *window);
	static void ProcessEvent();
	static bool WindowShouldClose();
	static void SetActiveCamera(Camera* camera);
	//static void ProcessKeyEvent();
	//static void ProcessMouseEvent();
private:
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


private:
	static GLFWwindow* currentWidnow;
	static Camera* activeCamera;

};