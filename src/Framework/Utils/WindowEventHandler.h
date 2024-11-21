#pragma once

#include "GLFW/glfw3.h"
#include <functional>
#include <map>
#include <string>

enum EventType {
	KEY_W_PRESS,
	KEY_S_PRESS,
	KEY_A_PRESS,
	KEY_D_PRESS,
	KEY_UP_PRESS,
	KEY_DOWN_PRESS,
	KEY_LEFT_PRESS,
	KEY_RIGHT_PRESS,
	KEY_I_PRESS

};

class WindowEventHandler {

public:
	static void BindWindow(GLFWwindow *window);
	static void ProcessEvent();
	static bool WindowShouldClose();
	static void SetEventCallBack(EventType eventType, std::function<void()> callback,const std::string& actionDescription);
	//static void ProcessKeyEvent();
	//static void ProcessMouseEvent();
private:
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


private:
	static GLFWwindow* currentWidnow;
	static std::map< EventType, std::function<void()>> s_eventCallBacks;
};