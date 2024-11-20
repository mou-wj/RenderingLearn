#include "WindowEventHandler.h"
#include "Camera.h"
GLFWwindow* WindowEventHandler::currentWidnow = nullptr;
Camera* WindowEventHandler::activeCamera = nullptr;
void WindowEventHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        // ������������¼�
}
void WindowEventHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // ��������¼�
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
        glfwSetWindowShouldClose(window, GLFW_TRUE);  // ���� ESC �رմ���
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
        //���Ͽ��൱�����е�������ת����z->y,��AROUND_X_NEGATIVE
        activeCamera->Rotate(RotateAction::AROUND_X_NEGATIVE);
        break;
    }
    case GLFW_KEY_DOWN: {
        activeCamera->Rotate(RotateAction::AROUND_X_POSITIVE);
        break;
    }
    case GLFW_KEY_RIGHT: {
        //���ҿ��൱�����е�������ת����x->z����AROUND_Y_POSITIVE
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
