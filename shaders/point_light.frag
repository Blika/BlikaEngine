#version 450

layout(location=0) in vec2 frag_offset;
layout(location=0) out vec4 out_color;

struct PointLight{
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 inv_view_matrix;
	vec4 ambient_light_color;
    PointLight point_lights[10];
    int lights;
} ubo;

layout(push_constant) uniform Push{
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.1415926538;

void main(){
    float dis = sqrt(dot(frag_offset,frag_offset));
    if(dis >= 1.0){
        discard;
    }
    float cos_dis = 0.5 * (cos(dis * M_PI) + 1.0);
    out_color = vec4(push.color.xyz + cos_dis, cos_dis);
}