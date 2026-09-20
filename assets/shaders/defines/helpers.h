float near = 1;
float far  = 100.0;

float LinearizeDepth(float depth)
{
    // float z = depth * 2.0 - 1.0; // back to NDC
    return (near * far) / (near + depth * (far - near));
}

vec3 UnprojectPos(vec3 pos, mat4 view, mat4 projection) {
    mat4 viewinv = inverse(view);
    mat4 projinv = inverse(projection);
    vec4 unprjpos =  viewinv * projinv * vec4(pos, 1.0);
    return unprjpos.xyz / unprjpos.w;
}
float ClipSpaceDepth(vec3 pos, mat4 proj, mat4 view) {
    vec4 clip = proj * view * vec4(pos.xyz, 1.0);
    return (clip.z / clip.w);
}

float LinearDepth(vec3 pos, mat4 proj, mat4 view) {
    float clipdepth = ClipSpaceDepth(pos, proj, view) ;//* 2.0 - 1.0; //[-1;1]
    // float depth = (2.0 * near * (far / 1)) / ((far / 1) + near - clipdepth * ((far / 1) - near)); // linear value between [0.01;10]
    return LinearizeDepth(clipdepth) / far;//depth / (far / 1);
}
