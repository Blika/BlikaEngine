#include "window.hpp"

#include <stdexcept>
#include <stb_image.h>

namespace blikaengine{

	Window::Window(int w, int h, std::string n): width{w}, height{h}, window_name{n}{
		init_window();
	}

	Window::~Window(){
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	
	void Window::create_window_surface(VkInstance instance, VkSurfaceKHR *surface){
		if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
			throw std::runtime_error("failed to create window surface");
		}
	}
	
	void Window::framebuffer_resize_callback(GLFWwindow* window, int width, int height){
		auto be_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		be_window->framebuffer_resized = false;
		be_window->width = width;
		be_window->height = height;
	}

	void Window::init_window(){
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);

		GLFWimage images[1];

		images[0].pixels = stbi_load("textures/blikaengine.jpg", &images[0].width, &images[0].height, 0, 4);
		glfwSetWindowIcon(window,1,images); 
		stbi_image_free(images[0].pixels);

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, framebuffer_resize_callback);
	}
}	