module;
#include "aai/utils/lookup_table.h"

export module aai.json.helper;
export import nlohmann.json;
export import glm;

export {
    using njson = nlohmann::json;
    namespace aai {
        namespace json {
#define ENTRIES(X) \
            X(COLOR_BLEND, "color_blend") \
            X(RT_REF, "rt_ref") \
            X(VIEWPORT, "viewport") \
            X(SCISSOR, "scissor") \
            X(DYNAMIC_VIEWPORT, "dynamic_viewport") \
            X(RASTERIZER, "rasterizer") \
            X(VERTEX, "vertex") \
            X(FRAGMENT, "fragment") \
            X(PIPELINE_REF, "pipeline_ref") \
            X(DEPTH_TEST, "depth_test") \
            X(DEPTH_WRITE, "depth_write") \
            X(MULTISAMPLING, "multisampling") \
            X(DEPTH_OP, "depth_op") \
            X(POLYGON, "polygon") \
            X(CULL, "cull") \
            X(FACE, "face") \
            X(SRC_COLOR, "src_color") \
            X(DST_COLOR, "dst_color") \
            X(SRC_ALPHA, "src_alpha") \
            X(DST_ALPHA, "dst_alpha") \
            X(C_OP, "c_op") \
            X(A_OP, "a_op") \
            X(BLEND, "blend") \
            X(SIZE, "size") 
DECLARE_LOOKUP_TABLE(ENTRIES, val)

        template <typename T>
        T get(const njson& d, json::val e, T default_val) {
            std::string val = json::get_value(e);
            if (d.contains(val))
                return d[val].get<T>();
            return default_val;
        }
        glm::vec2 get_vec2(const njson& d, json::val e) {
            glm::vec2 v(0);
            int i = 0;
            std::string val = json::get_value(e);
            if (!d.contains(val))
                return v; 
            const njson& vec_json = d[val];
            for (auto& k : vec_json) {
                if (i >= 2)
                    return v;
                v[i] = k.get<float>();
                i++;
            }
            return v;
        }
    }
} 
};
