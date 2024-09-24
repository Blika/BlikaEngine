#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <map>
#include <stdexcept>
#include <vector>

#include <iostream>

namespace blikaengine{
	
	struct PointLightPushConstant{
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout): device{device}{
		create_pipeline_layout(global_set_layout);
		create_pipeline(render_pass);
	}
	
	PointLightSystem::~PointLightSystem(){
		vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
	}

	void PointLightSystem::create_pipeline_layout(VkDescriptorSetLayout global_set_layout){
		
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(PointLightPushConstant);

		std::vector<VkDescriptorSetLayout> descriptor_sets_layouts{global_set_layout};

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_sets_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_sets_layouts.data();
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if(vkCreatePipelineLayout(device.device(),&pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS){
			throw std::runtime_error("failed to create pipeline layout");
		}
	}
	
	void PointLightSystem::create_pipeline(VkRenderPass render_pass){
		assert(pipeline_layout != VK_NULL_HANDLE && "cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config{};
		Pipeline::default_pipeline_config_info(pipeline_config);
		Pipeline::enable_alpha_blending(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout;
		be_pipeline = std::make_unique<Pipeline>(device, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv", pipeline_config);
	}
	void PointLightSystem::update(FrameInfo& frame_info, GlobalUbo& ubo){
		auto rotate = glm::rotate(glm::mat4(1.f), frame_info.frame_time, {0.f, -1.f, 0.f});
		int i = 0;
		for(auto& kv: frame_info.game_objects){
			auto& obj = kv.second;
			if(obj.point_light == nullptr) continue;
			assert(i < MAX_LIGHTS && "point lights exceed maximum specified");

			obj.transform.translation = glm::vec3(rotate * glm::vec4(obj.transform.translation,1.f));
			//if(obj.next_color == 0){
				obj.color.x += 0.25f * frame_info.frame_time * obj.multipliers[0];
				if(obj.color.x > 1.f){
					obj.color.x = 1.f;
					obj.multipliers[0] = -1;
				}else if(obj.color.x < 0.f){
					obj.color.x = 0.f;
					obj.multipliers[0] = 1;
					//obj.next_color = 1;
				}
			/*}else if(obj.next_color == 1){
				obj.color.y += 0.25f * frame_info.frame_time * obj.multipliers[1];
				if(obj.color.y > 1.f){
					obj.color.y = 1.f;
					obj.multipliers[1] = -1;
				}else if(obj.color.y < 0.f){
					obj.color.y = 0.f;
					obj.multipliers[1] = 1;
					obj.next_color = 2;
				}
			}else if(obj.next_color == 2){
				obj.color.z += 0.25f *frame_info.frame_time * obj.multipliers[2];
				if(obj.color.z > 1.f){
					obj.color.z = 1.f;
					obj.multipliers[2] = -1;
				}else if(obj.color.z < 0.f){
					obj.color.y = 0.f;
					obj.multipliers[2] = 1;
					obj.next_color = 0;
				}
			}*/
			obj.point_light->light_intensity += 0.1f * frame_info.frame_time * obj.multipliers[3];
			if(obj.point_light->light_intensity > 1.f){
				obj.point_light->light_intensity = 1.f;
				obj.multipliers[3] = -1;
			}else if(obj.point_light->light_intensity < 0.1f){
				obj.point_light->light_intensity = 0.1f;
				obj.multipliers[3] = 1;
			}

			ubo.point_lights[i].position = glm::vec4(obj.transform.translation,1.f);
			ubo.point_lights[i].color = glm::vec4(obj.color, obj.point_light->light_intensity);
			i++;
		}
		ubo.lights = i;
	}

	void PointLightSystem::render(FrameInfo& frame_info){
		std::map<float, GameObject::id_t> sorted;
		for(auto& kv: frame_info.game_objects){
			auto& obj = kv.second;
			if(obj.point_light == nullptr) continue;
			auto offset = frame_info.camera.get_position() - obj.transform.translation;
			float dis_squared = glm::dot(offset, offset);
			sorted[dis_squared] = obj.getId();
		}

		be_pipeline->bind(frame_info.command_buffer);
		auto projection_view = frame_info.camera.get_projection() * frame_info.camera.get_view();

		vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,0,1,&frame_info.global_descriptor_set,0,nullptr);
		for(auto it = sorted.rbegin(); it != sorted.rend(); ++it){
			auto& obj = frame_info.game_objects.at(it->second);
			//if(obj.point_light == nullptr) continue;
			PointLightPushConstant push{};
			push.position = glm::vec4(obj.transform.translation,1.f);
			push.color = glm::vec4(obj.color, obj.point_light->light_intensity);
			push.radius = obj.transform.scale.x;
			vkCmdPushConstants(frame_info.command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstant), &push);
        	vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
		}
	}
}