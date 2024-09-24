#pragma once

#include "camera.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"
#include "frame_info.hpp"

#include <memory>
#include <vector>

namespace blikaengine{

	class PointLightSystem{
		public:

			PointLightSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
			~PointLightSystem();
			PointLightSystem(const PointLightSystem&) = delete;
			PointLightSystem& operator = (const PointLightSystem&) = delete;
			
			void update(FrameInfo& frame_info, GlobalUbo& ubo);
			void render(FrameInfo& frame_info);

		private:
			void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
			void create_pipeline(VkRenderPass render_pass);

			Device& device;
			std::unique_ptr<Pipeline> be_pipeline;
			VkPipelineLayout pipeline_layout;
	};

}