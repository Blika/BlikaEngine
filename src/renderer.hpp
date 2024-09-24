#pragma once
#include "device.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace blikaengine{

	class Renderer{
		public:

			VkCommandBuffer primary_command_buffer;
			struct SecondaryCommandBuffers{
				VkCommandBuffer background;
				VkCommandBuffer ui;
			} secondary_command_buffers;

			Renderer(Window& window, Device& device);
			~Renderer();
			Renderer(const Renderer&) = delete;
			Renderer& operator = (const Renderer&) = delete;

			VkRenderPass get_swap_chain_render_pass() const{
				return swap_chain->getRenderPass();
			}

			float get_aspect_ratio() const{
				return swap_chain->extentAspectRatio();
			}

			bool is_frame_in_progress() const{
				return is_frame_started;
			}

			VkCommandBuffer get_current_command_buffer() const{
				assert(is_frame_started && "cannot get command buffer when frame is not in progress");
				return command_buffers[current_frame_index];
			}

			VkCommandBuffer begin_frame();
			void end_frame();
			void begin_swap_chain_render_pass(VkCommandBuffer command_buffer);
			void end_swap_chain_render_pass(VkCommandBuffer command_buffer);

			int get_frame_index()const{
				assert(is_frame_started && "cannot get frame index when frame is not in progress");
				return current_frame_index;
			}

		private:
			void create_command_buffers();
			void free_command_buffers();
			void recreate_swap_chain();

			Window& window;
			Device& device;
			std::unique_ptr<SwapChain> swap_chain;
			std::vector<VkCommandBuffer> command_buffers;

			uint32_t current_image_index;
			int current_frame_index{0};
			bool is_frame_started{false};
	};

}