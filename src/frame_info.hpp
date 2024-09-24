#pragma once

#include "camera.hpp"
#include "game_object.hpp"

#include <vulkan/vulkan.h>

namespace blikaengine{
	
	#define MAX_LIGHTS 10

	struct PointLight{
		glm::vec4 position{};
		glm::vec4 color{};
	};

	//alignas(16)
	struct GlobalUbo{
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::mat4 inverse_view{1.f};
		glm::vec4 ambient_light_color{1.f,1.f,1.f,0.02f};
		PointLight point_lights[MAX_LIGHTS];
		int lights;
	};
	
	struct FrameInfo{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		Camera& camera;
		VkDescriptorSet global_descriptor_set;
		GameObject::Map& game_objects;
	};

}