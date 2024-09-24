#include "master_render_system.hpp"
#include "blikaengine.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace blikaengine{
	
	struct SimplePushConstantData{
		glm::mat4 model_matrix{1.f};
		glm::mat4 normal_matrix{1.f};
	};
	
	MasterRenderSystem::MasterRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout): device{device}{
		create_pipeline_layout(global_set_layout);
		create_pipeline(render_pass);
	}
	
	MasterRenderSystem::~MasterRenderSystem(){
		vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
	}

	void MasterRenderSystem::create_pipeline_layout(VkDescriptorSetLayout global_set_layout){
		
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);

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
	
	void MasterRenderSystem::create_pipeline(VkRenderPass render_pass){
		assert(pipeline_layout != VK_NULL_HANDLE && "cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config{};
		Pipeline::default_pipeline_config_info(pipeline_config);
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout;
		be_pipeline = std::make_unique<Pipeline>(device, "shaders/master_shader.vert.spv", "shaders/master_shader.frag.spv", pipeline_config);
	}

	void MasterRenderSystem::render_game_objects(FrameInfo& frame_info){
		be_pipeline->bind(frame_info.command_buffer);
		auto projection_view = frame_info.camera.get_projection() * frame_info.camera.get_view();

		vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,0,1,&frame_info.global_descriptor_set,0,nullptr);

		for(auto& kv : frame_info.game_objects){
			auto& obj = kv.second;
			if(obj.model == nullptr) continue;
			SimplePushConstantData push{};
			push.model_matrix = obj.transform.mat4();
			push.normal_matrix = obj.transform.normal_matrix();
			vkCmdPushConstants(frame_info.command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0 ,sizeof(SimplePushConstantData), &push);
			obj.model->bind(frame_info.command_buffer);
			obj.model->draw(frame_info.command_buffer);
		}
	}

}