#include "WindowEventHandler.h"
#include "Camera.h"
#include <iostream>
GLFWwindow* WindowEventHandler::currentWidnow = nullptr;
std::map< EventType, std::function<void()>> WindowEventHandler::s_eventCallBacks;
void WindowEventHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (s_eventCallBacks[MOUSE_LEFT_BUTTON_CLICKED]) {
            s_eventCallBacks[MOUSE_LEFT_BUTTON_CLICKED]();
        }
        

    }
        // 处理左键按下事件
}
void WindowEventHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // 处理滚轮事件
    if (s_eventCallBacks[MOUSE_SCROLL_FORWARD])
    {
        s_eventCallBacks[MOUSE_SCROLL_FORWARD]();
    }
    if (s_eventCallBacks[MOUSE_SCROLL_BACK])
    {
        s_eventCallBacks[MOUSE_SCROLL_BACK]();
    }


}

void WindowEventHandler::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    switch (key)
    {
    case GLFW_KEY_ESCAPE: {
        if (action != GLFW_PRESS)
        {
            return;
        }
        glfwSetWindowShouldClose(window, GLFW_TRUE);  // 按下 ESC 关闭窗口
        break;
    }
    case GLFW_KEY_W: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_W_PRESS])
        {
            s_eventCallBacks[KEY_W_PRESS]();
        }


        break;
    }
    case GLFW_KEY_S: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_S_PRESS])
        {
            s_eventCallBacks[KEY_S_PRESS]();
        }
        break;
    }
    case GLFW_KEY_A: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_A_PRESS])
        {
            s_eventCallBacks[KEY_A_PRESS]();
        }
        break;
    }
    case GLFW_KEY_D: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_D_PRESS])
        {
            s_eventCallBacks[KEY_D_PRESS]();
        }
        break;
    }
    case GLFW_KEY_UP: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_UP_PRESS])
        {
            s_eventCallBacks[KEY_UP_PRESS]();
        }
        break;
    }
    case GLFW_KEY_DOWN: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_DOWN_PRESS])
        {
            s_eventCallBacks[KEY_DOWN_PRESS]();
        }
        break;
    }
    case GLFW_KEY_RIGHT: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_RIGHT_PRESS])
        {
            s_eventCallBacks[KEY_RIGHT_PRESS]();
        }

        break;
    }
    case GLFW_KEY_LEFT: {
        if (action == GLFW_REPEAT && s_eventCallBacks[KEY_LEFT_PRESS])
        {
            s_eventCallBacks[KEY_LEFT_PRESS]();
        }

        break;
    }
    case GLFW_KEY_I: {
        if (action == GLFW_PRESS && s_eventCallBacks[KEY_I_PRESS])
        {
            s_eventCallBacks[KEY_I_PRESS]();
        }

        break;
    }
    case GLFW_KEY_J: {
        if (action == GLFW_PRESS && s_eventCallBacks[KEY_J_PRESS])
        {
            s_eventCallBacks[KEY_J_PRESS]();
        }

        break;
    }
    case GLFW_KEY_K: {
        if (action == GLFW_PRESS && s_eventCallBacks[KEY_K_PRESS])
        {
            s_eventCallBacks[KEY_K_PRESS]();
        }

        break;
    }
    case GLFW_KEY_C: {
        if (action == GLFW_PRESS && s_eventCallBacks[KEY_C_PRESS])
        {
            s_eventCallBacks[KEY_C_PRESS]();
        }

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

void WindowEventHandler::SetEventCallBack(EventType eventType, std::function<void()> callback,const std::string& actionDescription)
{
    s_eventCallBacks[eventType] = callback;
    std::cout << actionDescription << std::endl;
}

std::array<float, 2> WindowEventHandler::GetMousePos()
{
    int window_x, window_y;
    glfwGetWindowPos(currentWidnow, &window_x, &window_y);
    double x, y;
    glfwGetCursorPos(currentWidnow, &x, &y);
    // 计算相对窗口的坐标
    double relative_x = x - window_x;
    double relative_y = y - window_y;

    return std::array<float, 2>{float(x), float(y)};


}
