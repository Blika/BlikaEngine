#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace blikaengine{

	class Window{
		public:
			Window(int w, int h, std::string n);
			~Window();

			Window(const Window&) = delete;
			Window& operator = (const Window&) = delete;

			bool should_close(){
				return glfwWindowShouldClose(window);
			}

			void create_window_surface(VkInstance instance, VkSurfaceKHR *surface);

			bool was_window_resized(){
				return framebuffer_resized;
			}
			void reset_window_resized_flag(){
				framebuffer_resized = false;
			}
			GLFWwindow* get_GLFWWindow() const{
				return window;
			}

			VkExtent2D get_extent(){
				return {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
			}

		private:
			static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
			void init_window();

			int width;
			int height;
			bool framebuffer_resized = false;
			std::string window_name;
			GLFWwindow* window;
	};

}