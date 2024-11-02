#pragma once

#include "GLFW/glfw3.h"


class WindowEventHandler {

public:
	static void BindWindow(GLFWwindow *window);
	static void ProcessEvent();
	static bool WindowShouldClose();
	//static void ProcessKeyEvent();
	//static void ProcessMouseEvent();



private:
	static GLFWwindow* currentWidnow;


};