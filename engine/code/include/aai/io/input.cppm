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
                    A = 1 << 3,
                    SHIFT = 1 << 4,
                };
                int state = 0;
                inline bool is_state(val bit) { return (state & static_cast<int>(bit)) != 0; }
                inline void clear_state(val bit) { (state &= ~(static_cast<int>(bit))); }
                inline void toogle_state(val bit) { (state ^= static_cast<int>(bit)); }
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
                            case GLFW_KEY_LEFT_SHIFT:
                                state |= int(val::SHIFT);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case GLFW_RELEASE: {
                        switch (key) {
                            case GLFW_KEY_W:
                                clear_state(val::W);
                                break;
                            case GLFW_KEY_S:
                                clear_state(val::S);
                                break;
                            case GLFW_KEY_A:
                                clear_state(val::A);
                                break;
                            case GLFW_KEY_D:
                                clear_state(val::D);
                                break;
                            case GLFW_KEY_LEFT_SHIFT:
                                clear_state(val::SHIFT);
                                break;
                            default:
                                state = 0;
                                break;
                        }
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
                    glm::vec2 tmp_delta;
                    bool mouse_moved = false;
                };
                data info;
                const glm::vec2& get_delta() { return info.delta; }
                const glm::vec2& get_pos() { return info.pos; }
                inline bool is_state(val bit) { return (info.state & static_cast<int>(bit)) != 0; }
                bool was_moved() { return info.mouse_moved; }
                void pos_callback(GLFWwindow* window, double xpos, double ypos) {
                    info.pos = { xpos, ypos };
                    if (is_state(aai::io::mouse::val::LEFT_PRESSED)) {
                        info.tmp_delta = info.delta;
                        info.delta = info.pressed_pos - info.pos;
                        info.mouse_moved = info.tmp_delta != info.delta;
                        info.pressed_pos = info.pos;
                    }
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
                            info.delta = {0, 0};
                            info.mouse_moved = false;
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
            }
        }
    }
};
