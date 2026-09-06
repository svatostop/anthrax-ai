export module aai.json.helper;

export {
    namespace aai {
        namespace json {
#define ENTRIES \
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

#define X(element, name) element,
            typedef enum {
                ENTRIES
            } val;
#undef X
            const char* get_value(const val id)
            {
                const char* retval;
#define X(element, name) if (id == element) { retval = name; } else
                ENTRIES
#undef X
                {
                    retval = "undef";
                }
                return retval;
            }
            val get_key(const char* id)
            {
                val retval;
#define X(element, name) if (id == name) { retval = element; } else
                ENTRIES
#undef X
                {
                    retval = SIZE;
                }
                return retval;
            }
        }
    } 
};
