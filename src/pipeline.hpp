#pragma once

#include "device.hpp"

#include <string>
#include <vector>

namespace blikaengine{

	struct PipelineConfigInfo{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> binding_descriptions{};
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		uint32_t subpass = 0;
	};

	class Pipeline{
		public:
			Pipeline(Device& device, const std::string& vert_filepath, const std::string& frag_filepath, const PipelineConfigInfo& config_info);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline() = default; 
			Pipeline& operator = (const Pipeline&) = delete;

			void bind(VkCommandBuffer command_buffer);
			
			static void default_pipeline_config_info(PipelineConfigInfo& configInfo);
			static void enable_alpha_blending(PipelineConfigInfo& configInfo);

		private:
			static std::vector<char> read_file(const std::string& file_path);
			void create_graphics_pipeline(const std::string& vert_filepath, const std::string& frag_filepath, const PipelineConfigInfo& config_info);

			void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

			Device& device;
			VkPipeline graphics_pipeline;
			VkShaderModule vert_shader_module;
			VkShaderModule frag_shader_module;
	};
}