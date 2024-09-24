#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_pos;
layout(location = 2) in vec3 frag_normal;
layout(location = 3) in vec2 frag_UV;

layout(location = 0) out vec4 out_color;

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

layout(set = 0, binding = 1) uniform sampler2D image;

layout(push_constant) uniform Push {
	mat4 model_matrix;
	mat4 normal_matrix;
} push;

void main(){
	vec3 diffuse_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
	vec3 specular_light = vec3(0.0);
	vec3 surface_normal = normalize(frag_normal);
	vec3 camera_pos_world = ubo.inv_view_matrix[3].xyz;
	vec3 view_direction = normalize(camera_pos_world - frag_pos);
	for(int i = 0; i < ubo.lights; i++){
		PointLight light = ubo.point_lights[i];
		vec3 direction_to_light = light.position.xyz - frag_pos;
		float attenuation = 1.0 / dot(direction_to_light,direction_to_light);
		direction_to_light = normalize(direction_to_light);
		float cos_angle_incidence =  max(dot(surface_normal, direction_to_light),0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;
		diffuse_light += intensity * cos_angle_incidence;

		//specular
		vec3 half_angle = normalize(direction_to_light + view_direction);
		float blinn_term = dot(surface_normal, half_angle);
		blinn_term = clamp(blinn_term, 0, 1);
		blinn_term = pow(blinn_term, 1024.0);
		specular_light += intensity * blinn_term;
	}
	vec3 image_color = texture(image, frag_UV).rgb;
	out_color = vec4((diffuse_light * frag_color + specular_light * frag_color) * image_color, 1);
}