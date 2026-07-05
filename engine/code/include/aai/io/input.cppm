module;
#include "aai/io/win_defines.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
export module aai.io;
import glm;
export {
    namespace  aai {
        namespace io {
            namespace key {
                enum class val {
                    W = 1 << 0,
                    S = 1 << 1,
                    D = 1 << 2,
                    A = 1 << 3
                };
                int state = 0;
                inline bool is_state(val bit) { return (state & static_cast<int>(bit)) != 0; }
                inline void clear_state(int bit) { (state &= ~bit); }
                inline void toogle_state(int bit) { (state ^= bit); }
                void callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
                switch (action) {
                    case GLFW_PRESS: {
                        switch (key) {
                            case GLFW_KEY_W:
                                state |= int(val::W);
                                break;
                            case GLFW_KEY_S:
                                state |= int(val::S);
                                break;
                            case GLFW_KEY_A:
                                state |= int(val::A);
                                break;
                            case GLFW_KEY_D:
                                state |= int(val::D);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case GLFW_RELEASE: {
                         state = 0;
                        break;
                    }
                    default:
                        break;
                    } 
                }
            }

            namespace mouse {
                enum class val {
                    LEFT_RELEASED = 1 << 0,
                    LEFT_PRESSED = 1 << 1,
                };
                struct data {
                    int state = 0;
                    glm::vec2 pos;
                    glm::vec2 pressed_pos;
                    glm::vec2 delta;
                };
                data info;
                const glm::vec2& get_delta() { return info.delta; }
                inline bool is_state(val bit) { return (info.state & static_cast<int>(bit)) != 0; }
                void pos_callback(GLFWwindow* window, double xpos, double ypos) {
                    info.pos = { xpos, ypos };
                    info.delta = info.pressed_pos - info.pos;
                }
                void button_callback(GLFWwindow* window, int button, int action, int mods) {
                    switch (action) {
                        case GLFW_PRESS: {
                            switch (button) {
                                case GLFW_MOUSE_BUTTON_LEFT: {
                                    info.pressed_pos = info.pos;
                                    info.state |= int(val::LEFT_PRESSED);
                                    break;
                                }
                                default:
                                    break;
                            }
                            break;
                        }
                        case GLFW_RELEASE: {
                            info.state = 0;
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
    }
};
