#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_pos;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec2 frag_UV;

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

layout(push_constant) uniform Push {
	mat4 model_matrix;
	mat4 normal_matrix;
} push;

void main(){
	vec4 position_world = push.model_matrix * vec4(position, 1.0f);
	gl_Position = ubo.projection_matrix * (ubo.view_matrix * position_world);
	frag_normal = normalize(mat3(push.normal_matrix) * normal);
	frag_pos = position_world.xyz;
	frag_color = color;
  	frag_UV = uv;
}