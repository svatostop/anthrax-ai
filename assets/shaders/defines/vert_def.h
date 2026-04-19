layout (location = 0) in vec4 vposition;
layout (location = 1) in vec4 vnormal;
layout (location = 2) in vec4 vcolor;
layout (location = 3) in vec4 vuv;

layout (location = 0) out vec4 outpos;
layout (location = 1) out vec4 outnormal;
layout (location = 2) out vec4 outcolor;
layout (location = 3) out vec4 outcoord;
out gl_PerVertex {
    vec4 gl_Position;
};
