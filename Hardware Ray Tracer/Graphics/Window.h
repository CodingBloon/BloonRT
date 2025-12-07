#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

namespace Core {
	class Window {

	public:
		Window(int width, int height, std::string title);
		~Window();

		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		void initWindow();
		void resizeWindow();

	private:
		int height, width;
		std::string title;

		GLFWwindow* window;
	};
}