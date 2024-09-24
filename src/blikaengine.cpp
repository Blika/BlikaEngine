#include "blikaengine.hpp"
#include "camera.hpp"
#include "render/master_render_system.hpp"
#include "render/point_light_system.hpp"
#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "texture.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>
#include <vector>

#include <iostream>

namespace blikaengine{

	BlikaEngine* BlikaEngine::instance = nullptr;
	std::default_random_engine BlikaEngine::rnd_eng(std::random_device{}());
	
	BlikaEngine::BlikaEngine(){
		global_pool = DescriptorPool::Builder(device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
          	.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		load_game_objects();
		BlikaEngine::instance = this;
	}
	
	BlikaEngine::~BlikaEngine(){
	}

	void BlikaEngine::run(){
		std::vector<std::unique_ptr<Buffer>> ubo_buffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for(int i = 0; i < ubo_buffers.size(); i++){
			ubo_buffers[i] = std::make_unique<Buffer>(device,sizeof(GlobalUbo),1,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			ubo_buffers[i]->map();
		}

		auto global_set_layout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		Texture texture = Texture(device, "textures/blikaengine.jpg");
		VkDescriptorImageInfo image_info{};
		image_info.sampler = texture.get_sampler();
		image_info.imageView = texture.get_image_view();
		image_info.imageLayout = texture.get_image_layout();

		std::vector<VkDescriptorSet> global_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for(int i = 0; i < global_descriptor_sets.size(); i++){
			auto buffer_info = ubo_buffers[i]->descriptorInfo();
			DescriptorWriter(*global_set_layout, *global_pool)
				.writeBuffer(0,&buffer_info)
				.writeImage(1,&image_info)
				.build(global_descriptor_sets[i]);
		}

		MasterRenderSystem master_render_system{device, renderer.get_swap_chain_render_pass(), global_set_layout->getDescriptorSetLayout()};
		PointLightSystem point_light_system{device, renderer.get_swap_chain_render_pass(), global_set_layout->getDescriptorSetLayout()};
		Camera camera{};
		auto viewer_object = GameObject::create_game_object();
		viewer_object.transform.translation.y = -1.f;
		viewer_object.transform.translation.z = -2.f;
		KeyboardMovementController camera_controller{};
		auto current_time = std::chrono::high_resolution_clock::now();
		while(!window.should_close()){
			glfwPollEvents();
			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
			current_time = new_time;
			camera_controller.move_in_plane_XZ(window.get_GLFWWindow(), frame_time, viewer_object);
			camera.set_view_YXZ(viewer_object.transform.translation, viewer_object.transform.rotation);
			float aspect = renderer.get_aspect_ratio();
			camera.set_orthographic_projection(-aspect,aspect,-1,1,-1,1);
			camera.set_perspective_projection(glm::radians(50.f), aspect, .1f, 100.f);
			if(auto command_buffer = renderer.begin_frame()){
				int frame_index = renderer.get_frame_index();
				FrameInfo frame_info{frame_index,frame_time,command_buffer,camera,global_descriptor_sets[frame_index],game_objects};

				//update
				GlobalUbo ubo{};
				ubo.projection = camera.get_projection();
				ubo.view = camera.get_view();
				ubo.inverse_view = camera.get_inverse_view();
				point_light_system.update(frame_info,ubo);
				ubo_buffers[frame_index]->writeToBuffer(&ubo);
				ubo_buffers[frame_index]->flush();

				//render
				renderer.begin_swap_chain_render_pass(command_buffer);

				master_render_system.render_game_objects(frame_info);
				point_light_system.render(frame_info);

				renderer.end_swap_chain_render_pass(command_buffer);
				renderer.end_frame();
			}
		}
		vkDeviceWaitIdle(device.device());
	}

	void BlikaEngine::load_game_objects(){
		std::shared_ptr<Model> model_floor = Model::create_model_from_file(device, "models/quad.obj");
		auto floor = GameObject::create_game_object();
		floor.model = model_floor;
		floor.transform.translation = {.0f, .0f, .0f};
		floor.transform.scale = {5.f, 1.f, 5.f};
		game_objects.emplace(floor.getId(), std::move(floor));

		std::shared_ptr<Model> model_mei = Model::create_model_from_file(device, "models/raiden_mei.obj");
		auto mei = GameObject::create_game_object();
		mei.model = model_mei;
		mei.transform.translation = {.0f, .0f, .0f};
		mei.transform.scale = {.1f, .1f, .1f};
		game_objects.emplace(mei.getId(), std::move(mei));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
  		};
		for(int i = 0; i < lightColors.size(); i++){
			auto light = GameObject::make_point_light(0.1f);
			light.color = lightColors[i];
			auto rotate = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), {0.f, -1.f, 0.f});
			light.transform.translation = glm::vec3(rotate * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			game_objects.emplace(light.getId(), std::move(light));
		}
	}
}