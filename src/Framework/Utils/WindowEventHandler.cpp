#include "WindowEventHandler.h"
#include "Camera.h"
GLFWwindow* WindowEventHandler::currentWidnow = nullptr;
Camera* WindowEventHandler::activeCamera = nullptr;
void WindowEventHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        // 处理左键按下事件
}
void WindowEventHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // 处理滚轮事件
}

void WindowEventHandler::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
    {
        return;
    }
    if (activeCamera == nullptr)
    {
        return;
    }
    switch (key)
    {
    case GLFW_KEY_ESCAPE: {
        glfwSetWindowShouldClose(window, GLFW_TRUE);  // 按下 ESC 关闭窗口
        break;
    }
    case GLFW_KEY_W: {
        activeCamera->Move(MoveDirection::FORWARD);
        break;
    }
    case GLFW_KEY_S: {
        activeCamera->Move(MoveDirection::BACK);
        break;
    }
    case GLFW_KEY_A: {
        activeCamera->Move(MoveDirection::LEFT);
        break;
    }
    case GLFW_KEY_D: {
        activeCamera->Move(MoveDirection::RIGHT);
        break;
    }
    case GLFW_KEY_UP: {
        //往上看相当于所有点往下旋转，即z->y,即AROUND_X_NEGATIVE
        activeCamera->Rotate(RotateAction::AROUND_X_NEGATIVE);
        break;
    }
    case GLFW_KEY_DOWN: {
        activeCamera->Rotate(RotateAction::AROUND_X_POSITIVE);
        break;
    }
    case GLFW_KEY_RIGHT: {
        //往右看相当于所有点往左旋转，即x->z，即AROUND_Y_POSITIVE
        activeCamera->Rotate(RotateAction::AROUND_Y_POSITIVE);
        break;
    }
    case GLFW_KEY_LEFT: {
        activeCamera->Rotate(RotateAction::AROUND_Y_NEGATIVE);
        break;
    }
    default:
        break;
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

void WindowEventHandler::SetActiveCamera(Camera* camera)
{
    activeCamera = camera;
}
