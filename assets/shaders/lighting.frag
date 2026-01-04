#version 450

#include "defines/defines.h"
#include "defines/fragdef.h"

//#extension GL_EXT_debug_printf : enable
const float constant = 1.0f;
const float linear = 0.7f;
const float quadratic = 1.8f;

struct LightInfo {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 DirLight(LightInfo light, vec3 normal, vec3 view_dir, vec3 albedo)
{
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64.0f);
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec ;
    return ambient + diffuse + specular;
} 

vec3 PointLight(LightInfo light, vec3 normal, vec3 position, vec3 view_dir, vec3 albedo)
{
    vec3 light_dir = normalize(light.position - position);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 16.0f);
    float distance = length(light.position - position);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance)) * 2;    

    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo * light.color;
    vec3 specular = light.specular * spec ;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
} 

void main()
{
    //debugPrintfEXT("%f|%f", gl_FragCoord.x, gl_FragCoord.y);
    bool has_cube = GetResource(Camera, GetUniformInd()).hascubemap;
    bool has_shadows = GetResource(Camera, GetUniformInd()).hasshadows;
    int cube_ind = GetResource(Camera, GetUniformInd()).cubemapbind;
    vec3 sky = vec3(0);
    // if (has_cube) {
    //     vec2 sky_uv = inpos.xy ;
    //     sky_uv.y *= -1.0;//sky_uv.y;
    //     sky = texture(cubemaps[cube_ind], vec3(sky_uv, 1)).xyz;
    // }
    vec3 result = sky;
    vec3 cam_pos = GetResource(Camera, GetUniformInd()).viewpos.xyz;

    vec3 diffuse = GetResource(Camera, GetUniformInd()).diffuse.xyz;
    vec3 specular = GetResource(Camera, GetUniformInd()).specular.xyz;
    vec3 ambient = GetResource(Camera, GetUniformInd()).ambient.xyz;

    vec3 glob_light_dir = GetResource(Camera, GetUniformInd()).global_light_dir.xyz;

    vec2 uv = incoord.xy;
    uv.y = 1.0 - uv.y;
    
    vec4 normal = texture(textures[GetTextureInd()], uv.xy);
    vec4 position = texture(textures[GetTextureInd() + 1], uv.xy );
    normal = normalize(normal);
    vec3 albedo = texture(textures[GetTextureInd() + 2], uv.xy).xyz;
    
    float shadow = 0.0;
    // shadows
    if (has_shadows)
    {
        const mat4 bias = mat4( 
             0.5, 0.0, 0.0, 0.0,
             0.0, 0.5, 0.0, 0.0,
             0.0, 0.0, 1.0, 0.0,
             0.5, 0.5, 0.0, 1.0 );

        vec4 lightpos = bias * GetResource(Camera, GetUniformInd()).shadow_matrix * vec4(position.xyz, 1);
        vec4 shadow_coord = lightpos / lightpos.w;
        float current_depth = shadow_coord.z;  
        float depth_bias = 0.0001;
        vec2 texelSize = 1.0 / textureSize(textures[GetTextureInd() + 3], 0);
        for(int x = -1; x <= 1; ++x)
        {
        for(int y = -1; y <= 1; ++y)
        {
            float closest_depth = texture(textures[GetTextureInd() + 3], shadow_coord.xy + vec2(x, y) * texelSize).r; 
            shadow += current_depth - depth_bias > closest_depth ? 1.0 : 0.0;        
        }    
        }
        shadow /= 9.0;
    }

    vec3 view_dir = normalize(cam_pos - position.xyz);
    vec3 cubemap = vec3(1);
    if (has_cube) {
        vec3 I = normalize(position.xyz - cam_pos);
        vec3 R = reflect(I, normalize(normal.xyz));

        cubemap = texture(cubemaps[cube_ind], R).xyz;
    }
    vec3 p = position.xyz;
    LightInfo dirLight = { vec3(0), normalize(vec3(0) - glob_light_dir), vec3(1), ambient, diffuse, specular };
    
    vec3 dirlight = DirLight(dirLight, normal.xyz, view_dir, albedo);
    if (dirlight.x > 0 && dirlight.y > 0 && dirlight.z > 0) {
        result = dirlight * (1.0 - shadow * 0.8);
    }
    int point_size = GetResource(Camera, GetUniformInd()).point_light_size;
    int j = 0;
    for (int i = 0; i < point_size; i++) {
        if (j >= 4) {
            j = 0;
        }
        vec3 point_lights = GetResource(Camera, GetUniformInd()).point_light_pos[i].xyz;
        vec3 point_color  = GetResource(Camera, GetUniformInd()).point_light_color[i].xyz;
        float point_radius = GetResource(Camera, GetUniformInd()).point_light_radius[i][j];
        j++;
        LightInfo point = { point_lights, vec3(0), point_color, ambient, diffuse, specular };
        result += PointLight(point, normal.xyz, position.xyz, view_dir, albedo) * 10.0 ;
   // debugPrintfEXT("%d|||%f|%f|---||%f|%f|%f-----%f|%f|%f\n",GetTextureInd() + 1, uv.x, uv.y, position.r, position.g, position.b, albedo.x, albedo.y, albedo.z);
    }
    result = clamp(result , vec3(0), vec3(1));
    if (result.x <= 0 && result.y <= 0 && result.z <= 0 ) {
        discard;
    }
    outfragcolor = vec4(result  * cubemap, 1);//outfragcolor = vec4(position.xyz / 10.0, 1.0);
}
