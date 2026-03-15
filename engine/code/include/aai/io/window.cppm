module;
#include "aai/io/win_defines.h"

export module aai.window;

export {
    namespace  aai {
        class window {
            public:
                void init();
                void clean();

                bool closed() const { return glfwWindowShouldClose(win); }
                void poll_events() const { glfwPollEvents(); }

                Display* get_display() { return display; }
                Window get_x11_win() { return x11_win; }
            private:
                GLFWwindow* win;
                Display* display;
                Window x11_win;
        };                      
    }
};
