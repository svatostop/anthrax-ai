module;
#include "aai/io/win_defines.h"

module aai.window;
import aai.io;

void aai::window::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    win = glfwCreateWindow(800, 600, "aai", nullptr, nullptr);    
    //if (glfwGetPlatformProperties() & GLFW_PLATFORM_WAYLAND_BIT) {
    //}
    glfwSetKeyCallback(win, aai::io::key::callback);
    glfwSetCursorPosCallback(win, aai::io::mouse::pos_callback);
    glfwSetMouseButtonCallback(win, aai::io::mouse::button_callback);
    display = glfwGetX11Display();
    x11_win = glfwGetX11Window(win);
}

void aai::window::clean()
{
    glfwDestroyWindow(win);
    glfwTerminate();
}
