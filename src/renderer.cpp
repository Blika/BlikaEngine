#include "renderer.hpp"
#include "blikaengine.hpp"

#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace blikaengine{
	
	Renderer::Renderer(Window& window, Device& device): window{window}, device{device} {
		recreate_swap_chain();
		create_command_buffers();
	}
	
	Renderer::~Renderer(){
		free_command_buffers();
	}

	void Renderer::recreate_swap_chain(){
		auto extent = window.get_extent();
		while(extent.width == 0 || extent.height == 0){
			extent = window.get_extent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device.device());
		if(swap_chain == nullptr){
			swap_chain = std::make_unique<SwapChain>(device, extent);
		}else{
			std::shared_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
			swap_chain = std::make_unique<SwapChain>(device, extent, old_swap_chain);
			if(!old_swap_chain->compare_swap_formats(*swap_chain.get())){
				throw std::runtime_error("swap chain image/depth format has changed");
			}
		}
	}

	void Renderer::create_command_buffers(){
		command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device.getCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		if(vkAllocateCommandBuffers(device.device(),&alloc_info, command_buffers.data())!= VK_SUCCESS){
			throw std::runtime_error("failed to allocate command buffers");
		}
	}
	
	void Renderer::free_command_buffers(){
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
		command_buffers.clear();
	}

	VkCommandBuffer Renderer::begin_frame(){
		assert(!is_frame_started && "can't call begin_frame while in progress");
		auto result = swap_chain->acquireNextImage(&current_image_index);
		if(result == VK_ERROR_OUT_OF_DATE_KHR){
			recreate_swap_chain();
			return nullptr;
		}
		if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
			throw std::runtime_error("failed to acquire swap change image");
		}
		is_frame_started = true;
		auto command_buffer = get_current_command_buffer();
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if(vkBeginCommandBuffer(command_buffer,&begin_info) != VK_SUCCESS){
			throw std::runtime_error("command buffer failed to begin recording");
		}
		return command_buffer;
	}

	void Renderer::end_frame(){
		assert(is_frame_started && "can't call end_frame while not in progress");
		auto command_buffer = get_current_command_buffer();
		if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS){
			throw std::runtime_error("failed to record command buffer");
		}
		auto result = swap_chain->submitCommandBuffers(&command_buffer,&current_image_index);
		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_window_resized()){
			window.reset_window_resized_flag();
			recreate_swap_chain();
		}else if(result != VK_SUCCESS){
			throw std::runtime_error("failed to present swap chain image");
		}
		is_frame_started = false;
		current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::begin_swap_chain_render_pass(VkCommandBuffer command_buffer){
		assert(is_frame_started && "can't call begin_swap_chain_render_pass while not in progress");
		assert(command_buffer == get_current_command_buffer() && "can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = swap_chain->getRenderPass();
		render_pass_info.framebuffer = swap_chain->getFrameBuffer(current_image_index);
		render_pass_info.renderArea.offset = {0,0};
		render_pass_info.renderArea.extent = swap_chain->getSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {0.01f,0.01f,0.01f,1.0f};
		clear_values[1].depthStencil = {1.0f,0};

		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swap_chain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swap_chain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0,0}, swap_chain->getSwapChainExtent()};
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}
	void Renderer::end_swap_chain_render_pass(VkCommandBuffer command_buffer){
		assert(is_frame_started && "can't call end_swap_chain_render_pass while not in progress");
		assert(command_buffer == get_current_command_buffer() && "can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(command_buffer);
	}
}