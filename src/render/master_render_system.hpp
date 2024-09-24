#pragma once

#include "camera.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"
#include "frame_info.hpp"

#include <memory>
#include <vector>

namespace blikaengine{

	class MasterRenderSystem{
		public:

			MasterRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
			~MasterRenderSystem();
			MasterRenderSystem(const MasterRenderSystem&) = delete;
			MasterRenderSystem& operator = (const MasterRenderSystem&) = delete;

			void render_game_objects(FrameInfo& frame_info);

		private:
			void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
			void create_pipeline(VkRenderPass render_pass);

			Device& device;
			std::unique_ptr<Pipeline> be_pipeline;
			VkPipelineLayout pipeline_layout;
	};

}