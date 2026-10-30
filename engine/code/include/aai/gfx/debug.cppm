module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.debug;

import aai.utils;
import aai.gfx.vk.gpu_data;

import std;
import glm;
export {
    namespace gfx {
        namespace dbg {
            enum primitive_t {
                POINT = 0,
                LINE,
                SIZE
            };

            class draw : public utils::singleton<draw> {
                public:
                    void submit(primitive_t type, const glm::vec3& start, const glm::vec3& end = {}, const glm::mat4& m = glm::mat4(1.0f)) {
                        std::lock_guard<std::mutex> lock(datas_mtx);
                        std::vector<point>& v = datas[type];
                        v.push_back(point(start, m));
                        if (type != POINT) 
                            v.push_back(point(end, m));
                    }
                    void submit(primitive_t type, const glm::vec3& start, const glm::mat4& m = glm::mat4(1.0f)) {
                        std::lock_guard<std::mutex> lock(datas_mtx);
                        std::vector<point>& v = datas[type];
                        v.push_back(point(start, m));
                    }

                    void reset() {
                        std::lock_guard<std::mutex> lock(datas_mtx);
                        datas.clear();
                    }
                    void take(std::map<primitive_t, std::vector<point>>& out) {
                        std::lock_guard<std::mutex> lock(datas_mtx);
                        out.swap(datas);
                    }
                    void set_vk_bind(primitive_t type, VkDeviceAddress addr) { binds[type] = addr; }
                    VkDeviceAddress get_vk_bind(primitive_t type) { return binds[type]; }

                    bool enabled() { return is_enabled; }
                    void set_enabled(bool b) { is_enabled = b; }
                    float get_global_anim_mult() { return global_anim_mult; }
                    void set_global_anim_mult(float f) { global_anim_mult = f; }

                private:                    
                    std::map<primitive_t, VkDeviceAddress> binds;
                    mutable std::mutex datas_mtx;
                    std::map<primitive_t, std::vector<point>> datas;
                    bool is_enabled = false;
                    float global_anim_mult = 1.0f;
            };
        }
    }
};

