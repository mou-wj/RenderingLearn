#include "WindowEventHandler.h"

GLFWwindow* WindowEventHandler::currentWidnow = nullptr;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        // ������������¼�
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // ��������¼�
}
bool s_enableInteractor = false;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);  // ���� ESC �رմ���
    if (key == GLFW_KEY_I && action == GLFW_PRESS )
    {
        s_enableInteractor = true;
    }
    if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        s_enableInteractor = false;
    }
}



void WindowEventHandler::BindWindow(GLFWwindow* window)
{
    currentWidnow = window;
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

}

void WindowEventHandler::ProcessEvent()
{
    glfwPollEvents();
}

bool WindowEventHandler::WindowShouldClose()
{
    return glfwWindowShouldClose(currentWidnow);
}
